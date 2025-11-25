// Simple printf logging for Arduino-ish code.

#pragma once

#include <Arduino.h>
#include <cstdarg>
#include <cstdint>

//
// Logging context should be declared at file toplevel:
// static OkLoggingContext OK_CONTEXT("my_tag");
//

enum OkLoggingLevel : int8_t {
  OK_DETAIL_LEVEL,
  OK_NOTE_LEVEL,
  OK_ERROR_LEVEL,
  OK_FATAL_LEVEL,
};

// Logging macros expect an OkLoggingContext named OK_CONTEXT in scope
struct OkLoggingContext {
  char const* tag;
  OkLoggingLevel min;
  OkLoggingContext(char const* tag);   // Initializes tag and minimum level
};

//
// Logging macros for different priority levels
//

#define OK_DETAIL(fmt, ...) OK_REPORT(OK_DETAIL_LEVEL, fmt, ##__VA_ARGS__)
#define OK_NOTE(fmt, ...) OK_REPORT(OK_NOTE_LEVEL, fmt, ##__VA_ARGS__)
#define OK_ERROR(fmt, ...) OK_REPORT(OK_ERROR_LEVEL, fmt, ##__VA_ARGS__)
#define OK_FATAL(fmt, ...) OK_REPORT_SOURCE(OK_FATAL_LEVEL, fmt, ##__VA_ARGS__)
#define OK_FATAL_IF(c) if (c) OK_FATAL("%s", #c) else {}
#define OK_ERROR_IF(c) \
    ({const auto e = c; if (e) OK_REPORT_SOURCE(OK_ERROR_LEVEL, "%s", #c); e;})

#define OK_LOGGABLE(l) ((l) >= OK_CONTEXT.min && (l) >= ok_logging_minimum)
#define OK_REPORT(lev, fmt, ...) if (OK_LOGGABLE(lev)) \
    ok_log(OK_CONTEXT.tag, (lev), "" fmt, ##__VA_ARGS__); else {}
#define OK_REPORT_SOURCE(lev, fmt, ...) if (OK_LOGGABLE(lev)) \
    ok_log(OK_CONTEXT.tag, (lev), "" fmt "\n  at: %s:%d\n  in: %s", \
           ##__VA_ARGS__, __FILE__, __LINE__, __PRETTY_FUNCTION__); else {}

void ok_log(char const* tag, OkLoggingLevel, char const*, ...);
void ok_logv(char const* tag, OkLoggingLevel, char const*, va_list);

//
// Logging configuration
//

// Define this global constant as a comma-separated list of "tag=level"
// selectors with "*" wildcards, eg. "mymodule=spam,subsystem.*=problem,*=note".
extern char const* const ok_logging_config;

// Assign to set a global squelch on top of tag config, default OK_DETAIL_LEVEL
extern OkLoggingLevel ok_logging_minimum;

// Assign to change the logging output Stream, default &Serial
extern Print* ok_logging_stream;

// Assign to your own function to redefine logging strategy entirely.
// The function is called after filtering and printf argument formatting:
//   tag - name of tag generating the log message
//   lev - logging level of message
//   ts - millis() timestamp on first processing log
//   text - log message after printf argument formatting is done
using OkLoggingFunction =
    void(char const* tag, OkLoggingLevel lev, uint32_t ts, char const* text);
extern OkLoggingFunction* ok_logging_function;

//
// Cross-architecture serial initialization helper
//

struct OkLoggingSerialOptions {
  int baud = 115200;
  int rx_buffer_size = 0;
  int tx_buffer_size = 0;
  bool tx_non_blocking = false;
  int connect_wait_millis = 5000;  // If USB data is present, wait for monitor
};

// Calls Serial.begin(opts.baud) and then applies the other settings
// if supported by the current architecture.
void ok_serial_begin(OkLoggingSerialOptions const& = {});
