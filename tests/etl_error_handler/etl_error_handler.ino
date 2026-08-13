#include <ok_logging.h>
#include <etl/vector.h>

#if !ETL_USE_OK_LOGGING
#error ETL_USE_OK_LOGGING not set -- is etl_profile.h picked up?
#endif

#if !ETL_LOG_ERRORS
#error ETL_LOG_ERRORS not set -- is etl_profile.h picked up?
#endif

OkLoggingContext OK_CONTEXT("etl_error_handler");

static etl::vector<int, 2> vec;

void setup() {
  Serial.setTxBufferSize(4096);
  Serial.begin(115200);
  Serial.println("BEGIN-TEST");

  vec.push_back(1);

  // Out of bounds; etl::vector::at() asserts unconditionally, so this does not
  // depend on ETL_CHECK_PUSH_POP or friends. ETL calls the registered error
  // handler, which logs at OK_FATAL_LEVEL and therefore aborts. If the handler
  // never registered, ETL has nothing to call and execution just continues.
  (void) vec.at(5);

  Serial.println("TEST-FAIL: ETL error did not reach the ok_logging handler");
  Serial.flush();
  abort();  // end the run anyway, so the failure is a clean log mismatch
}

void loop() {
  Serial.println("TEST-FAIL: reached loop()");
  delay(1000);
}
