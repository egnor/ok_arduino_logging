#include <ok_logging.h>

#if !ARDUINO_ARCH_ESP32
#error not ESP32!
#elif ARDUINO_USB_CDC_ON_BOOT
#error unexpected USB!
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
