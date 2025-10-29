#include "ok_logging.h"

// Default logging config, in a separate file to support weak linkage.

#if defined OK_LOGGING_CONFIG
  extern char const* const ok_logging_config = #OK_LOGGING_CONFIG;
#else
  extern char const* const ok_logging_config __attribute__((weak)) = nullptr;
#endif
