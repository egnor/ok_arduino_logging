#include <ok_logging.h>

#if !ARDUINO_ARCH_ESP32
#error not ESP32!
#elif !ARDUINO_USB_CDC_ON_BOOT
#error not USB!
#elif !ARDUINO_USB_MODE
#error not serial/JTAG!
#elif ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(2, 0, 15)
#error old arduino-esp32 version!
#endif

OkLoggingContext OK_CONTEXT("serial_begin");

void setup() {
  ok_serial_begin({.tx_buffer_size = 4096});
  Serial.println("BEGIN-TEST");
  OK_NOTE("Note");
  Serial.flush();
  Serial.printf("TX-BUF=%d\r\n", Serial.availableForWrite());
  Serial.println("END-TEST");
}

void loop() {
  Serial.println("LOOP");
  delay(1000);
}
