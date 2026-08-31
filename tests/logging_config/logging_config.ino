#include <ok_logging.h>

#define STRINGIFY_(...) #__VA_ARGS__
#define STRINGIFY(...) STRINGIFY_(__VA_ARGS__)

#if defined(OK_LOGGING_CONFIG)
  #define OK_LOGGING_CONFIG_STR STRINGIFY(OK_LOGGING_CONFIG)
#else
  #define OK_LOGGING_CONFIG_STR "-undef-"
#endif

#if defined(TEST_LOGGING_CONFIG)
  char const* const ok_logging_config = STRINGIFY(TEST_LOGGING_CONFIG);
#endif

static OkLoggingContext OK_CONTEXT("logging_config");

void setup() {
  Serial.setTxBufferSize(4096);
  Serial.begin(115200);
  Serial.println("BEGIN-TEST");
  char const* const config = ok_logging_config ? ok_logging_config : "-null-";
  OK_NOTE("OK_LOGGING_CONFIG=%s", OK_LOGGING_CONFIG_STR);
  OK_NOTE("ok_logging_config=%s", config);

  {
    static OkLoggingContext OK_CONTEXT("aaa~~~bbb~~~ccc");
    Serial.printf("tag=%s min=%d\n", OK_CONTEXT.tag, OK_CONTEXT.min);
  }
  {
    static OkLoggingContext OK_CONTEXT("aaa~~~bbb~~~ccc~~~");
    Serial.printf("tag=%s min=%d\n", OK_CONTEXT.tag, OK_CONTEXT.min);
  }
  {
    static OkLoggingContext OK_CONTEXT("~~~aaa~~~bbb~~~ccc");
    Serial.printf("tag=%s min=%d\n", OK_CONTEXT.tag, OK_CONTEXT.min);
  }
  {
    static OkLoggingContext OK_CONTEXT("~~~aaa~~~bbb~~~ccc~~~");
    Serial.printf("tag=%s min=%d\n", OK_CONTEXT.tag, OK_CONTEXT.min);
  }

  OK_NOTE("sleeping 1sec");
  delay(1000);
  OK_FATAL("END-TEST");
}

void loop() {
  Serial.println("TEST-FAIL");  // should not be reached
  delay(1000);
}
