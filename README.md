# ok_arduino_logging

Simple diagnostic printf logging for Arduino-ish code on 32-bit+ processors, with support for module tagging and per-module level filtering.

You should first consider these more established libraries:
- [Arduino-Log](https://github.com/JSC-TechMinds/Arduino-Log) - level filtering, printf formatting, multiple `Print` outputs
- [arduino-logger](https://github.com/embeddedartistry/arduino-logger) - level filtering, printf formatting, ISR-safe, many buffer/output choices, some of which do level filtering for numbered modules
- [DebugLog](https://github.com/hideakitai/DebugLog) - level filtering, argument formatting, `Stream` and/or file output
- [EasyLogger](https://github.com/x821938/EasyLogger) - level filtering, module ("service") filtering, any `Print` output
- [ESPLogger](https://github.com/kuslota/esplogger) - per-module level filtering loggers, no formatting, serial output
- [Logger](https://github.com/bakercp/Logger) - level filtering, no formatting, serial output or custom function
- [MycilaLogger](https://github.com/mathieucarbou/MycilaLogger) - ESP32 only, per-module level filtering loggers, printf formatting, multiple `Print` outputs
- [SerialLogger](https://github.com/UltiBlox/SerialLogger) - combined data/diagnostic logging, no formatting, level filtering, serial output

This library (ok_arduino_logging) differs by supporting centrally configured name-based per-module log level filtering. Source files set a module name (arbitrary string like `network` or `imu-data` or `my_library.callbacks`) to attach to log statements in the file. The main app is compiled with a filter expression defining the log level for each module (like `network=detail,imu-data=error,my_library.*=notice`). The filter is evaluated at startup so log statements run quickly.

Other niceties include:
- printf formatting (arguments aren't evaluated if the log is suppressed)
- assert-like `OK_ERROR_IF(...)` and `OK_FATAL_IF(...)` macros
- source file, line, and function reported for fatal and assert-check errors, but not others
- compact emoji representations of log levels

These would be nice but are NOT supported:
- NO custom formatting of log lines
- NO routing to multiple destinations (eg. sd card + serial)
- NO management of I/O blocking from logging
- NO thread or ISR safety guarantees

## Minimal example

```
#include <ok_logging.h>

static OkLoggingContext OK_CONTEXT("my_app");

void setup() {
  Serial.begin(115200);  // or whatever
  OK_NOTE("In setup!");
}

void loop() {
  OK_NOTE("In loop!");
  delay(500);
}
```

There must be an `OkLoggingContext` object named `OK_CONTEXT` in scope for any logging macro use. This is normally a static global at file scope, as above. The object constructor takes the module name, which is any string descriptive of that section of code, or may be omitted.

## Reference

Available logging macros and functions:
- `OK_DETAIL(fmt, ...)` - lowest priority level
- `OK_NOTE(fmt, ...)` - "normal" priority level
- `OK_ERROR(fmt, ...)` - elevated priority, indicates a problem
- `OK_FATAL(fmt, ...)` - highest priority, aborts and reboots
- `OK_FATAL_IF(cond)` - if `cond` is `true`, reports source location and `#cond`, aborts and reboots
- `OK_ERROR_IF(cond)` - if `cond` is `true`, reports source location and `#cond`, returns the value of `cond` either way
- `set_ok_logging_output(output)` - redirects output to a `Print*` stream (`&Serial` by default)
- `set_ok_logging_global_minimum(level)` - sets a global level minimum level, and returns the old global level (default `OK_DETAIL_LEVEL`)

## Logging level configuration

Logging levels are set with a configuration string, which can be set one of two ways:
- Define `char const* ok_logging_config = "...";` in a source file
- OR pass `-DOK_LOGGING_CONFIG=...` to the compiler

Either way, the configuration string is a comma-separated series of `moduleglob=level` expressions, where `moduleglob` is the name of a module (as used to initialize `OK_CONTEXT`) or a pattern with `*` wildcards, and `level` is a logging level such as `DETAIL`, `NOTE` or `ERROR`. A bare `level` with no `=` matches all modules, equivalent to `*=level`. The first matching rule is used for any given module.

For example, the configuration "`foo=DETAIL,bar=ERROR,baz.*=NOTICE,FATAL`":
- shows all messages for module "`foo`"
- only `ERROR` or `FATAL` for "`bar`"
- `NOTICE` or higher for any module name starting with "`baz.`" (eg. "`baz.blah`", but not "`baz`" with no period)
- only `FATAL` for anything else

## Considerations

Logging macros synchronously write to the output object (`Serial` by default). On most Arduino runtimes, serial output is thread-safe but not interrupt-safe.

The logging formatter attempts to use a 128-byte stack buffer first (so 128 bytes should be available on the stack), if the message would exceed that size a dynamically allocated buffer is used, if allocation fails an error message is printed instead.
