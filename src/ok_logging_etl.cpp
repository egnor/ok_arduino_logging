// Adapter for the Embedded Template Library (ETL) to use ok_logging.h macros

#if defined(__has_include) && __has_include("etl/error_handler.h")
#include "etl/error_handler.h"  // will pull in etl_profile.h, etc.
#endif

// Must be set in etl_profile.h or elsewhere to enable
#if ETL_USE_OK_LOGGING

#include "ok_logging.h"
#include "Embedded_Template_Library.h"

static void etl_error_handler(etl::exception const& e) {
  ok_log(
      "ETL", OK_FATAL_LEVEL, "%s\n  at: %s:%d",
      e.what(), e.file_name(), e.line_number());
}

bool ok_logging_register_with_etl() {
  etl::error_handler::set_callback<etl_error_handler>();
  return true;
}

#else

bool ok_logging_register_with_etl() { return false; }

#endif
