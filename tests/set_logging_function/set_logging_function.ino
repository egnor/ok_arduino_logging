#include <ok_logging.h>

OkLoggingContext OK_CONTEXT("set_logging_function");

static void alt_logging_function(
  char const* tag,
  OkLoggingLevel lev,
  uint32_t millis,
  char const* text
) {
  Serial.printf(
    "ALT: tag=\"%s\" lev=%d millis=%ld text=\"%s\"\n",
    tag, lev, millis, text
  );
}

void setup() {
  Serial.begin(115200);
  Serial.println("BEGIN-TEST");
  OK_NOTE("Default log message");

  ok_logging_function = alt_logging_function;
  OK_NOTE("Alt function log message");
  OK_FATAL("Alt function fatal message");  // should abort after printing
}

void loop() {
  Serial.println("TEST-FAIL");  // should not be reached
  delay(1000);
}
