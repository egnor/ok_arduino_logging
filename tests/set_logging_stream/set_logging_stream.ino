#include <ok_logging.h>

OkLoggingContext OK_CONTEXT("set_logging_stream");

HardwareSerial alt_serial(1);

void setup() {
  Serial.begin(115200);
  alt_serial.begin(115200, SERIAL_8N1, 21, 19);

  Serial.println("BEGIN-TEST");
  Serial.println("Default serial port output");
  OK_NOTE("Default serial log message");

  alt_serial.println("BEGIN-TEST");
  ok_logging_stream = &alt_serial;
  OK_NOTE("Alt serial log message");
  OK_FATAL("Alt serial fatal message");
}

void loop() {
  Serial.println("TEST-FAIL");  // should not be reached
  delay(1000);
}
