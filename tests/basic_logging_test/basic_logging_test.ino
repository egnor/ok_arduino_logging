#include <ok_logging.h>

OkLoggingContext OK_CONTEXT("log_tag");

void setup() {
  Serial.begin(115200);
  OK_DETAIL("Detail");
  OK_NOTE("Note");
  OK_NOTE("\n\nNote with newlines before and after\n\n");
  OK_NOTE("Note with\nnewlines in the\nmiddle");
  OK_ERROR("Error");
  OK_ERROR("\n\nError with newlines before and after\n\n");
}

void loop() {
  Serial.println("+++SIMDONE");
  delay(1000);
}
