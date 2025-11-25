#include <ok_logging.h>

#if !defined(ARDUINO_ARCH_RP2040)
#error not RP2040!
#endif

OkLoggingContext OK_CONTEXT("serial_begin");

void setup() {
  ok_serial_begin();  // doesn't actually affect Serial1
  ok_logging_stream = &Serial1;
  Serial1.println("BEGIN-TEST");
  OK_NOTE("Note");
  Serial1.println("END-TEST");
}

void loop() {
  Serial1.println("LOOP");
  delay(1000);
}
