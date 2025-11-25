#include <ok_logging.h>

#if !defined(ARDUINO_ARCH_RP2040)
#error not RP2040!
#endif

OkLoggingContext OK_CONTEXT("serial_begin");

void setup() {
  ok_serial_begin();
  Serial.println("BEGIN-TEST");
  OK_NOTE("Note");
  Serial.println("END-TEST");
}

void loop() {
  Serial.println("LOOP");
  delay(1000);
}
