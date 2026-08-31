#include <ok_logging.h>

char const* const ok_logging_config = "DETAIL";

static OkLoggingContext OK_CONTEXT("");

void setup() {
  Serial.setTxBufferSize(4096);
  Serial.begin(115200);
  Serial.println("BEGIN-TEST");
  OK_DETAIL("detail");
  OK_NOTE("note");
  OK_ERROR("error");
  OK_FATAL("fatal");
}

void loop() {
  Serial.println("TEST-FAIL");  // should not be reached
  delay(1000);
}
