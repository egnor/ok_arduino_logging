#include <ok_logging.h>

OkLoggingContext OK_CONTEXT("log_tag");

void setup() {
  Serial.begin(115200);
  OK_DETAIL("Detail message");
  OK_DETAIL("");
  OK_NOTE("Note message");
  OK_NOTE("\n\nNote with newlines before and after\n\n");
  OK_NOTE("Note with\nnewlines in the\nmiddle");
  OK_NOTE("");
  OK_ERROR("Error message");
  OK_ERROR("");
  OK_ERROR("\n\nError with newlines before and after\n\n");
  OK_FATAL("Fatal message");
}

void loop() {
  Serial.println("FAIL");  // should not be reached
  delay(1000);
}
