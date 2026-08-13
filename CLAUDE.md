# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

An Arduino library (`ok_arduino_logging`) providing printf-style diagnostic logging with per-tag level filtering. Targets 32-bit+ Arduino-ish MCUs — currently exercised against ESP32 (incl. ESP32-C3) and RP2040. Distributed as a normal Arduino library (`library.properties`, `dot_a_linkage=true`).

The public API lives entirely in `src/ok_logging.h`. The README is the authoritative user-facing doc — read it before changing API surface.

## Architecture

- **`OkLoggingContext`** is intended as a file-scoped static named `OK_CONTEXT`. Its constructor walks the global `ok_logging_config` string (comma-separated `tagpattern=level` rules, `*` wildcards, first match wins, default `NOTE`) and caches the resolved minimum level. All logging macros (`OK_DETAIL` / `OK_NOTE` / `OK_ERROR` / `OK_FATAL` / `OK_ERROR_IF` / `OK_FATAL_IF`) expand to an `if (OK_LOGGABLE(...))` guard against `OK_CONTEXT.min`, so suppressed calls never evaluate their printf args. The macros require `OK_CONTEXT` to be visible at the call site.

- **`ok_logging_config`** is a `char const* const` global with weak linkage in `ok_logging_config.cpp`. Users override it either by defining `char const* const ok_logging_config = "..."` in their own TU (strong wins over weak) or by passing `-DOK_LOGGING_CONFIG=...` (the `.cpp` stringifies the macro). Keep this file tiny and isolated so weak linkage continues to work.

- **`ok_log` / `ok_logv`** in `ok_logging.cpp` do printf formatting into a 128-byte stack buffer, falling back to `malloc` for longer messages (with explicit "[log allocation error]" fallback). They then call `ok_logging_function` (a swappable function pointer, default `default_logging_function` which emits to `ok_logging_stream`, default `&Serial`). `OK_FATAL_LEVEL` triggers `delay(1000); abort()` after the message is printed.

- **`ok_serial_begin`** (`src/ok_serial_begin.cpp`) is an unrelated convenience helper. It wraps `Serial.begin()` with per-architecture quirks (ESP32 HWCDC vs USBCDC vs UART; RP2040 arduino-pico). The `#if` ladder branches on `ARDUINO_ARCH_ESP32`, `ARDUINO_USB_CDC_ON_BOOT`, `ARDUINO_USB_MODE`, `ESP_ARDUINO_VERSION`, `ARDUINO_ARCH_RP2040`, `NO_USB`. Touch carefully — each branch corresponds to real hardware variants covered by a test in `tests/`.

- If **`ETL_USE_OK_LOGGING`** is defined, `ok_logging.cpp` registers a callback with the Embedded Template Library's error handler, which calls `ok_log()` with the ETL error message and `FATAL` severity level.

- **`src/ok_logging_esp32.h`** adds `OK_LOG_ESP_ERRORS` / `OK_FATAL_ESP_ERRORS` macros, only active under `ARDUINO_ARCH_ESP32`.

## Tests

Tests live in `tests/`, one subdirectory per scenario. Each test dir contains:

- A `.ino` sketch that prints `BEGIN-TEST` then exercises the feature.
- A `sketch.yaml` selecting the `fqbn` and platform (this is what distinguishes ESP32 vs ESP32-C3 vs RP2040 USB/non-USB).
- `diagram.json`, `wokwi.toml`, `scenario.yaml` for the [Wokwi](https://wokwi.com) simulator.
- A `*_test.py` that regex-matches the captured serial log.

The `tests/conftest.py` `wokwi_output_dir` fixture (module-scoped) shells out to `arduino-cli compile` and then `wokwi-cli` for the sketch in the test file's directory, writing `output.tmp/serial_log.txt`. Tests are simple regex assertions over that log.

### Running tests

Run from `tests/` with toolchain managed by `mise` + `uv`:

```sh
cd tests
mise install              # one-time: installs arduino-cli, wokwi-cli, python, uv
uv sync                   # one-time: installs pytest deps
uv run pytest             # run all tests
uv run pytest basic_logging/                              # one scenario
uv run pytest basic_logging/basic_logging_test.py::test_basic_logging -v  # one test
```

`mise.toml` pins `arduino-cli` (1.3.1), `wokwi-cli` (0.19.1), Python 3.12, and sets `ARDUINO_DIRECTORIES_DATA` to `tests/build.tmp/arduino` so Arduino cores/libs install under the repo rather than `~/.arduino15`. `WOKWI_CLI_TOKEN` is checked in (intentionally — it's the project token). The `postinstall` hook runs `arduino-cli update` and `uv sync`.

Wokwi runs against a cloud API, so tests need network; an occasional `API Error: ... code 1006` is a transient transport drop, not a real failure. Re-run before chasing it.

Build artifacts: each test dir gets an `output.tmp/` (compiled `.bin` + `serial_log.txt`); top-level toolchain state lives in `tests/build.tmp/`. Both are gitignored via `*.tmp`.

## Conventions

- `OK_REPORT_SOURCE` (used by `OK_FATAL*` and `OK_ERROR_IF`) appends `__FILE__:__LINE__` and `__PRETTY_FUNCTION__`. Plain `OK_NOTE`/`OK_ERROR` deliberately do *not* — keep that asymmetry.
- The default formatter trims trailing whitespace and rewrites embedded CR/LF as `println()` calls; tests assert this behavior (see `basic_logging_test.py`).
- Tag matching in `min_level_for_tag` / `glob_match` is case-insensitive (`strncasecmp`). Level names accept many aliases (`level_for_name` in `ok_logging.cpp`).
