#include "ok_logging.h"

#include <Arduino.h>
#include <cstdlib>
#include <cstring>

static OkLoggingLevel min_level_for_tag(char const*);
static OkLoggingFunction default_logging_function;

// Exposed globals
OkLoggingLevel ok_logging_minimum = OK_DETAIL_LEVEL;
Print* ok_logging_stream = &Serial;
OkLoggingFunction* ok_logging_function = &default_logging_function;

OkLoggingContext::OkLoggingContext(char const* tag)
  : tag(tag), min(min_level_for_tag(tag)) {}

void ok_log(char const* tag, OkLoggingLevel lev, char const* fmt, ...) {
  va_list va;
  va_start(va, fmt);
  ok_logv(tag, lev, fmt, va);
  va_end(va);
}

void ok_logv(char const* tag, OkLoggingLevel lev, char const* fmt, va_list va) {
  auto const t = millis();
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

  // Trim trailing newlines before calling logging output function
  while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) --len;
  buf[len] = '\0';
  ok_logging_function(tag, lev, t, buf);
  if (buf != stack_buf) free(buf);
}

static void default_logging_function(
  char const* tag, OkLoggingLevel lev, uint32_t millis, char const* text
) {
  if (ok_logging_stream == nullptr) return;

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
    ok_logging_stream->println(start);
  }

  if (lev == OK_FATAL_LEVEL) {
    ok_logging_stream->println("  🚨 REBOOT IN 1 SEC 🚨\n");
    ok_logging_stream->flush();
    delay(1000);
    abort();
  }
}

static char const* next_of(char const* p, char const* end, char const* m) {
  while (p < end && !strchr(m, *p)) ++p;
  return p;
}

static bool glob_match(
    char const* glob, char const* glob_end,
    char const* match, char const* match_end) {
  // Text before the first '*' must match exactly
  char const* next_star = next_of(glob, glob_end, "*");
  if (strncasecmp(match, glob, next_star - glob)) return false;
  if (next_star >= glob_end) return true;

  // Text between '*'s must match in order
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

  // Text after the last '*' must be a suffix
  return 
      match_end - match >= glob_end - glob &&
      strncasecmp(match_end - (glob_end - glob), glob, glob_end - glob) == 0;
}

static void trim(char const** p, char const** end) {
  while (*p < *end && strchr(" \t\n", **p)) ++*p;
  while (*p < *end && strchr(" \t\n", (*end)[-1])) --*end;
}

static OkLoggingLevel level_for_name(char const* level, char const* end) {
  auto const is = [level, end](char const* s) {
    return !strncasecmp(level, s, end - level) && !s[end - level];
  };

  if (is("*") ||
      is("a") || is("all") ||
      is("d") || is("debug") || is("detail") ||
      is("v") || is("verbose")) {
    return OK_DETAIL_LEVEL;
  }

  if (is("default") ||
      is("i") || is("info") ||
      is("n") || is("normal") || is("note") || is("notice") || is("notable")) {
    return OK_NOTE_LEVEL;
  }

  if (is("e") || is("error") ||
      is("p") || is("prob") || is("problem") ||
      is("w") || is("warn") || is("warning")) {
    return OK_ERROR_LEVEL;
  }

  if (is("none") || is("f") || is("fatal") || is("p") || is("panic")) {
    return OK_FATAL_LEVEL;
  }

  ok_log(
      "ok_logging", OK_ERROR_LEVEL, "Bad level \"%.*s\" in config:\n  %s",
      end - level, level, ok_logging_config);
  return OK_DETAIL_LEVEL;
}

static OkLoggingLevel min_level_for_tag(char const* tag) {
  if (ok_logging_config == nullptr) return OK_NOTE_LEVEL;

  // Find the "tag=level,..." entry that matches the given tag
  char const* config_end = ok_logging_config + strlen(ok_logging_config);
  char const* tag_end = tag ? tag + strlen(tag) : nullptr;
  char const* pos = ok_logging_config;
  while (true) {
    char const* entry = pos;
    char const* entry_end = next_of(pos, config_end, ",;");

    char const* glob = entry;
    char const* glob_end = next_of(entry, entry_end, "=:");

    char const* level = glob_end < entry_end ? glob_end + 1 : glob_end;
    char const* level_end = entry_end;

    trim(&glob, &glob_end);
    trim(&level, &level_end);
    if (glob == glob_end || level == level_end) {
      ok_log(
          "ok_logging", OK_ERROR_LEVEL, "Bad entry \"%.*s\" in config:\n  %s",
          level_end - glob, glob, ok_logging_config);
    } else if (glob_match(glob, glob_end, tag, tag_end)) {
      return level_for_name(level, level_end);
    }

    if (entry_end == config_end) return OK_NOTE_LEVEL;  // Default
    pos = entry_end + 1;
  }
}
