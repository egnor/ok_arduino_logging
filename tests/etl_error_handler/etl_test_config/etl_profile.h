// ETL build configuration for the ok_logging ETL test.
//
// ETL includes "etl_profile.h" from etl/platform.h, guarded by __has_include,
// so it will NOT find it in the sketch dir but WILL find it in a library.

#pragma once

#define ETL_LOG_ERRORS 1
#define ETL_VERBOSE_ERRORS 1
#define ETL_USE_OK_LOGGING 1
