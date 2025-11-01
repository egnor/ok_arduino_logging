# OK Logging for Arduino

Simple diagnostic printf logging for Arduino-ish code on 32-bit+ processors, with support for tagging and per-tag level filtering.

You should first consider these more established libraries:
- [Arduino-Log](https://github.com/JSC-TechMinds/Arduino-Log) - level filtering, printf formatting, multiple `Print` outputs
- [arduino-logger](https://github.com/embeddedartistry/arduino-logger) - level filtering, printf formatting, ISR-safe, many implementation choices
- [DebugLog](https://github.com/hideakitai/DebugLog) - level filtering, argument formatting, `Stream` and/or file output
- [EasyLogger](https://github.com/x821938/EasyLogger) - level filtering, tag ("service") filtering, any `Print` output
- [ESPLogger](https://github.com/kuslota/esplogger) - logger-object level filtering, no formatting, serial output
- [Logger](https://github.com/bakercp/Logger) - level filtering, no formatting, serial output or custom function
- [MycilaLogger](https://github.com/mathieucarbou/MycilaLogger) - logger-object level filtering, printf formatting, multiple `Print` outputs, ESP32 only
- [roo_logging](https://github.com/dejwk/roo_logging) - level filtering, per-tag verbosity, iostreams formatting, serial output
- [SerialLogger](https://github.com/UltiBlox/SerialLogger) - combined data/diagnostic logging, no formatting, level filtering, serial output

This library (ok_arduino_logging) adds **centrally configured string-tag log level filtering**: Each source file define a tag string which is added to logs from that file. The main app/sketch is compiled with a filter expression for tags and log levels, so you can dial verbosity up and down for per subsystem without modifying library code or rebuilding the world.

Log tags are free form strings: they can be the source file name, a logical subsystem name, the name of a library, or whatever else makes sense for filtering. Multiple source files can use the same tag, and tags can be localized to classes or functions. Filter expressions can use `*` wildcards for tag matching to allow for structured tags (like `network.http.parser`).

Importantly, libraries can use tags (often the library name, maybe with suffixes) so the app can tweak per-library log verbosity without modifying library code.

Other niceties include:
- printf formatting (arguments aren't evaluated if the log is suppressed)
- assert-like `OK_ERROR_IF(...)` and `OK_FATAL_IF(...)` macros for convenience
- source file, line, and function reporting for fatal and assert-check errors, but not others
- compact emoji representations of log levels
- [basic unit tests](tests) (!!) using [wokwi](https://wokwi.com/)

These would be nice but are NOT currently supported:
- custom formatting of log lines
- routing to multiple destinations (eg. sd card + serial)
- management/prevention of I/O blocking from logging
- management/prevention of repeated log-spam
- thread or ISR safety guarantees

## Minimal example

```
#include <ok_logging.h>

static OkLoggingContext OK_CONTEXT("my_app");
static int loop_count;

void setup() {
  Serial.begin(115200);  // or whatever
  OK_NOTE("In setup!");
  OK_ERROR("Oh no, an error occurred!");
}

void loop() {
  OK_NOTE("In loop! Counter=%d", loop_count);
  delay(500);
  ++loop_count;
}
```

Note, an `OkLoggingContext` object named `OK_CONTEXT` must be in scope for any logging macro use. This is typically a file-scoped static global, as above. The constructor takes the log tag (any string, or `nullptr`) for logging statements in this scope.

This example generates output like the following:

```
0.524 [my_app] In setup!
0.525 ⚠️ [my_app] Oh no, an error happened!
0.526 [my_app] In loop! Counter=0
1.996 [my_app] In loop! Counter=1

```

## Reference

Several macros and functions are defined in `<ok_logging.h>`
- `OK_DETAIL(fmt, ...)` - lowest priority level
- `OK_NOTE(fmt, ...)` - "normal" priority level
- `OK_ERROR(fmt, ...)` - elevated priority, indicates a problem
- `OK_FATAL(fmt, ...)` - highest priority, aborts and reboots
- `OK_FATAL_IF(cond)` - if `cond` is `true`, logs source location and `#cond`, aborts and reboots
- `OK_ERROR_IF(cond)` - if `cond` is `true`, logs source location and `#cond`; returns `cond` either way
- `set_ok_logging_output(output)` - redirects logs to a `Print*` stream (`&Serial` by default)
- `set_ok_logging_global_minimum(level)` - sets a global minimum level to print, and returns the old global minimum level (default `OK_DETAIL_LEVEL`)

## Log verbosity configuration

Logs are filtered with a global configuration string, which can be set one of two ways:
- Define `char const* ok_logging_config = "...";` in a source file
- OR pass `-DOK_LOGGING_CONFIG=...` to the compiler

Either way, the configuration string is a comma-separated series of `tagpattern=level` rules, where `tagpattern` is a log tag (as set in `OK_CONTEXT`) with optional `*` wildcards, and `level` is one of `DETAIL`, `NOTE`, `ERROR`, or `FATAL`. A bare level name with no `=` is equivalent to `*=level`. For each log tag, the first matching rule is used as the minimum level to print.

For example, the configuration "`foo=DETAIL,bar=ERROR,baz.*=NOTE,FATAL`":
- prints all logs tagged "`foo`"
- prints `ERROR` or `FATAL` logs tagged "`bar`"
- prints non-`DETAIL` logs with a tag starting with "`baz.`" (eg. "`baz.blah`" but not plain "`baz`"), 
- only prints `FATAL` logs with any other tag

When no rule matches a tag, or there is no configuration, the default is `NOTE` (everything but `DETAIL`).

## Considerations

Logging macros synchronously write to the output object (`Serial` by default) which can mess with timing. On most Arduino runtimes, serial output is thread-safe but not interrupt-safe.

The logging formatter attempts to use a 128-byte stack buffer first (so 128 bytes should be available on the stack), if the message would exceed that size a dynamically allocated buffer is used, if allocation fails an error message is printed instead.
