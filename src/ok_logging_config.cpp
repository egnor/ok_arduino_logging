#include "ok_logging.h"

// Default logging config, in a separate file to support weak linkage.

#if defined(OK_LOGGING_CONFIG)
  #define OK_LOGGING_STRINGIFY_(...) #__VA_ARGS__
  #define OK_LOGGING_STRINGIFY(...) OK_LOGGING_STRINGIFY_(__VA_ARGS__)
  extern char const* const ok_logging_config =
      OK_LOGGING_STRINGIFY(OK_LOGGING_CONFIG);
#else
  extern char const* const ok_logging_config __attribute__((weak)) = nullptr;
#endif
