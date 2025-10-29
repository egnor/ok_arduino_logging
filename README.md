# ok_arduino_logging

Simple diagnostic printf logging for Arduino-ish code on 32-bit+ processors, with support for module tagging and per-module level filtering.

You should first consider these more established libraries:
- [Arduino-Log](https://github.com/JSC-TechMinds/Arduino-Log) - level filtering, printf formatting, multiple `Print` outputs
- [arduino-logger](https://github.com/embeddedartistry/arduino-logger) - level filtering, printf formatting, ISR-safe, many buffer/output choices, some of which do level filtering for numbered modules
- [DebugLog](https://github.com/hideakitai/DebugLog) - level filtering, argument formatting, `Stream` and/or file output
- [EasyLogger](https://github.com/x821938/EasyLogger) - level filtering, module ("service") filtering, any `Print` output
- [ESPLogger](https://github.com/kuslota/esplogger) - per-module loggers with level filtering, no formatting, serial output
- [Logger](https://github.com/bakercp/Logger) - level filtering, no formatting, serial output or custom function
- [MycilaLogger](https://github.com/mathieucarbou/MycilaLogger) - ESP32 only, per-module loggers with level filtering, printf formatting, multiple `Print` outputs
- [SerialLogger](https://github.com/UltiBlox/SerialLogger) - combined data/diagnostic logging, no formatting, level filtering, serial output

This library (ok_arduino_logging) differs by supporting centrally configured name-based per-module log level filtering. Source files set a module name (arbitrary string like `network` or `imu-data` or `my_library.callbacks`) to attach to log statements in the file. The main app is compiled with a filter expression defining the log level for each module (like `network=detail,imu-data=error,my_library.*=notice`). The filter is evaluated at startup so log statements run quickly.

Other niceties include:
- printf formatting (with arguments not evaluated if the log statement is suppressed)
- assert-like `OK_ERROR_IF(...)` and `OK_FATAL_IF(...)` macros
- source file, line, and function reported for fatal and assert-check errors, but not others
- compact emoji representations of log levels

Things that would be nice but are NOT supported:
- custom formatting of log lines
- routing to multiple destinations (eg. sd card + serial)
- better management of I/O blocking from logging
- better thread and ISR safety guarantees
