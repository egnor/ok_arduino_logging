#include <ok_logging.h>

void setup() {
  Serial.setTxBufferSize(4096);
  Serial.begin(115200);
  Serial.println("BEGIN-TEST");
  {
    OkLoggingContext OK_CONTEXT("aaa~~~bbb~~~ccc");
    OK_DETAIL("shown");
  }
  {
    OkLoggingContext OK_CONTEXT("aaa~~~bbb~~~ccc~~~");
    OK_DETAIL("hidden");
    OK_NOTE("shown");
  }
  {
    OkLoggingContext OK_CONTEXT("~~~aaa~~~bbb~~~ccc");
    OK_NOTE("hidden");
    OK_ERROR("shown");
  }
  {
    OkLoggingContext OK_CONTEXT("~~~aaa~~~bbb~~~ccc~~~");
    OK_ERROR("hidden");
    OK_FATAL("shown");
  }
}

void loop() {
  Serial.println("TEST-FAIL");  // should not be reached
  delay(1000);
}
