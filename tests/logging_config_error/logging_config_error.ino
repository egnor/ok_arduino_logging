#include <ok_logging.h>

char const* const ok_logging_config = "foo=,note,baz";

static OkLoggingContext OK_CONTEXT("");

void setup() {
  Serial.setTxBufferSize(4096);
  Serial.begin(115200);
  Serial.println("BEGIN-TEST");
  OK_NOTE("note");
  Serial.println("END-TEST");
}

void loop() {
  delay(1000);
}
