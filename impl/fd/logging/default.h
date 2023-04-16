#pragma once

#include <fd/logging/logger.h>

namespace fd
{
using default_logger_t = logger<log_level::
#ifdef _DEBUG
                                    debug
#else
                                    err
#endif
                                >;
extern default_logger_t *const default_logger;
}