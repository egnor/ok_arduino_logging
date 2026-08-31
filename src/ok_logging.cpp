#include "ok_logging.h"

#include <Arduino.h>
#include <cstdlib>
#include <cstring>

#if defined(__has_include) && __has_include("etl/error_handler.h")
#include "etl/error_handler.h"  // Pulls in etl_profile.h, etc.
#endif

static OkLoggingLevel min_level_for_tag(char const*);
static OkLoggingFunction default_logging_function;

static char const* logging_config_error = nullptr;
static unsigned long last_complaint_millis = -10000;  // complain immediately

// Exposed globals
OkLoggingLevel ok_logging_minimum = OK_DETAIL_LEVEL;
Print* ok_logging_stream = &Serial;
bool ok_logging_non_blocking = false;
OkLoggingFunction* ok_logging_function = &default_logging_function;

OkLoggingContext::OkLoggingContext(char const* tag)
  : tag(tag), min(min_level_for_tag(tag)) {}

static char const* next_of(char const* p, char const* end, char const* m) {
  while (p < end && !strchr(m, *p)) ++p;
  return p;
}

static void trim(char const** p, char const** end) {
  while (*p < *end && strchr(" \t\r\n", **p)) ++*p;
  while (*p < *end && strchr(" \t\r\n", (*end)[-1])) --*end;
}

void ok_log(char const* tag, OkLoggingLevel lev, char const* fmt, ...) {
  va_list va;
  va_start(va, fmt);
  ok_logv(tag, lev, fmt, va);
  va_end(va);
}

void ok_logv(char const* tag, OkLoggingLevel lev, char const* fmt, va_list va) {
  auto const t = millis();
  if (logging_config_error && t - last_complaint_millis > 1000) {
    auto const* error = logging_config_error;
    auto const error_end = next_of(error, error + strlen(error), ",;");
    logging_config_error = nullptr;  // avoid recursion during complaint
    last_complaint_millis = t;  // complain periodically
    if (error == ok_logging_config && *error_end == '\0') {
      ok_log("ok_logging", OK_ERROR_LEVEL, "Bad config: \"%s\"", error);
    } else {
      ok_log(
        "ok_logging", OK_ERROR_LEVEL,
        "Bad directive: \"%.*s\"\n  Full config: \"%s\"",
        error_end - error, error, ok_logging_config
      );
    }
    logging_config_error = error;
  }

  char stack_buf[128];
  char* buf = stack_buf;
  auto len = vsnprintf(buf, sizeof(stack_buf), fmt, va);
  if (len < 0) {
    buf = strncpy(stack_buf, "[log formatting error]", sizeof(stack_buf));
    len = strlen(buf);
  } else if (len >= sizeof(stack_buf)) {
    buf = (char*) malloc(len + 1);
    if (buf == nullptr) {
      buf = strncpy(stack_buf, "[log allocation error]", sizeof(stack_buf));
      len = strlen(buf);
    } else {
      auto len2 = vsnprintf(buf, len + 1, fmt, va);
      if (len2 != len) {
        free(buf);
        buf = strncpy(stack_buf, "[log reformatting error]", sizeof(stack_buf));
        len = strlen(buf);
      }
    }
  }

  // Trim trailing whitespace before calling logging output function
  while (len > 0 && strchr(" \t\r\n", buf[len - 1])) --len;
  buf[len] = '\0';
  ok_logging_function(tag, lev, t, buf);
  if (lev == OK_FATAL_LEVEL) { delay(1000); abort(); }
  if (buf != stack_buf) free(buf);
}

static void print_log_entry(
  char const* tag, OkLoggingLevel lev, uint32_t millis, char const* text
) {
  // Allow one leading blank line before log text (suppress any others)
  auto const* start = text;
  if (*start == '\n' || *start == '\r') {
    do { ++start; } while (*start == '\n' || *start == '\r');
    ok_logging_stream->println();
  }

  // Allow blank nonerror log lines with no prefix
  if (*start == '\0' && lev <= OK_NOTE_LEVEL) {
    ok_logging_stream->println();
  } else {
    // Print log prefix
    ok_logging_stream->print(millis * 1e-3f, 3);
    switch (lev) {
      case OK_FATAL_LEVEL: ok_logging_stream->print(" 💥 "); break;
      case OK_ERROR_LEVEL: ok_logging_stream->print(" ⚠️ "); break;
      case OK_NOTE_LEVEL: ok_logging_stream->print(" "); break;
      case OK_DETAIL_LEVEL: ok_logging_stream->print(" 🕸️ "); break;
    }
    if (tag != nullptr && tag[0] != '\0') {
      ok_logging_stream->print("[");
      ok_logging_stream->print(tag);
      ok_logging_stream->print("] ");
    }
    if (lev == OK_FATAL_LEVEL) ok_logging_stream->print("FATAL ");
    if (lev >= OK_ERROR_LEVEL && *start == '\0') {
      ok_logging_stream->print("ERROR");
    }

    // Convert any CR/LF combination into println() (CR+LF)
    do {
      char const* eol = start;
      while (*eol && *eol != '\r' && *eol != '\n') ++eol;
      ok_logging_stream->write(start, eol - start);
      ok_logging_stream->println();
      if (*eol == '\r') ++eol;
      if (*eol == '\n') ++eol;
      start = eol;
    } while (*start != '\0');
  }

  if (lev == OK_FATAL_LEVEL) {
    ok_logging_stream->println("  🚨 REBOOT IN 1 SEC 🚨");
    ok_logging_stream->println();
    ok_logging_stream->flush();  // will delay() & abort() back in ok_logv()
  }
}

static void default_logging_function(
  char const* tag, OkLoggingLevel lev, uint32_t millis, char const* text
) {
  static uint32_t full_millis = 0;  // First log entry blocked by full buffer

  if (ok_logging_stream == nullptr) return;

  // FATAL always emits (and flush()/abort()s); backpressure check otherwise.
  // Static state tracks whether we've already noted the current drop run.
  if (lev != OK_FATAL_LEVEL && ok_logging_non_blocking) {
    int const avail = ok_logging_stream->availableForWrite();

    // 32 bytes for prefix (timestamp, emoji, brackets, "ERROR", etc),
    // plus prefix and body with an extra byte per line for CRLF translation
    int needed = 32 + (tag ? (int) strlen(tag) : 0) + strlen(text);
    for (char const* p = text; *p; ++p) {
      if (*p == '\r' || *p == '\n') ++needed;
    }
    if (needed > avail && full_millis == 0) full_millis = millis;
    if (full_millis != 0 && full_millis != (uint32_t) -1 && avail >= 32) {
      ok_logging_stream->print(full_millis * 1e-3f, 3);
      ok_logging_stream->println(" ⏸️ BUF FULL");
      full_millis = (uint32_t) -1;  // Sentinel to avoid re-printing marker
    }

    if (needed > avail) return;
    full_millis = 0;
  }

  print_log_entry(tag, lev, millis, text);
}

static bool glob_match(
    char const* glob, char const* glob_end,
    char const* match, char const* match_end) {
  // Glob-text before the first '*' must be a prefix
  char const* next_star = next_of(glob, glob_end, "*");
  if (strncasecmp(match, glob, next_star - glob)) return false;

  // Glob-text with no '*' must match directly
  if (next_star >= glob_end) return match_end - match == glob_end - glob;

  // Glob-text between '*'s must match in order
  while (true) {
    match += (next_star - glob);
    glob = next_star + 1;
    next_star = next_of(glob, glob_end, "*");
    if (next_star >= glob_end) break;

    char const* last_match = match_end - (next_star - glob);
    while (strncasecmp(match, glob, next_star - glob)) {
      if (++match > last_match) return false;
    }
  }

  // Glob-text after the last '*' must be a suffix
  return 
      match_end - match >= glob_end - glob &&
      strncasecmp(match_end - (glob_end - glob), glob, glob_end - glob) == 0;
}

static OkLoggingLevel level_for_name(char const* level, char const* end) {
  auto const is = [level, end](char const* s) {
    return !strncasecmp(level, s, end - level) && !s[end - level];
  };

  if (is("*") || is("d") || is("detail")) return OK_DETAIL_LEVEL;
  if (is("n") || is("note")) return OK_NOTE_LEVEL;
  if (is("e") || is("error")) return OK_ERROR_LEVEL;
  if (is("f") || is("fatal")) return OK_FATAL_LEVEL;

  // Defer error reporting until a log call when the runtime is set up
  if (!logging_config_error) logging_config_error = level;  // Init-safe error
  return OK_DETAIL_LEVEL;
}

static OkLoggingLevel min_level_for_tag(char const* tag) {
  if (ok_logging_config == nullptr) return OK_NOTE_LEVEL;

  // Find the "tag=level,..." entry that matches the given tag
  static char const* config_end = ok_logging_config + strlen(ok_logging_config);
  char const* tag_end = tag ? tag + strlen(tag) : nullptr;
  char const* pos = ok_logging_config;
  while (true) {
    char const* entry = pos, *entry_end = next_of(pos, config_end, ",;");
    char const* split = next_of(entry, entry_end, "=:");
    if (entry < split && split < entry_end) {
      char const* glob = entry, *glob_end = split;
      trim(&glob, &glob_end);
      if (glob_match(glob, glob_end, tag, tag_end)) {
        char const* level = split + 1, *level_end = entry_end;
        trim(&level, &level_end);
        return level_for_name(level, level_end);
      }
    } else if (split == entry_end && entry_end == config_end) {
      return level_for_name(entry, entry_end);
    } else {
      // Defer error reporting until a log call when the runtime is set up
      if (!logging_config_error) logging_config_error = entry;
    }
    if (entry_end == config_end) return OK_NOTE_LEVEL;  // Default
    pos = entry_end + 1;
  }
}

// Adapter for the Embedded Template Library (ETL) to report errors as OK_FATAL
// The user must define ETL_USE_OK_LOGGING=1 (eg. in etl_config.h) to enable
#if ETL_USE_OK_LOGGING
  #if !ETL_LOG_ERRORS
    #error "ETL_USE_OK_LOGGING requires ETL_LOG_ERRORS=1"
  #endif

  static void etl_error_handler(etl::exception const& e) {
    ok_log(
        "ETL", OK_FATAL_LEVEL, "%s\n  at: %s:%d",
        e.what(), e.file_name(), e.line_number());
  }

  bool ok_logging_register_with_etl() {
    etl::error_handler::set_callback<etl_error_handler>();
    return true;
  }

  [[maybe_unused]] static bool const _etl_reg = ok_logging_register_with_etl();
#endif
