#pragma once

#include <fd/logging/logger.h>

namespace fd
{
#ifdef FD_DEFAULT_LOG_LEVEL
#define DEFAULT_LOG_LEVEL log_level::FD_DEFAULT_LOG_LEVEL
#else
#ifdef _DEBUG
#define DEFAULT_LOG_LEVEL log_level::debug
#else
#define DEFAULT_LOG_LEVEL log_level::err
#endif
#endif

using default_logger_t = logger_same_level<DEFAULT_LOG_LEVEL, char, wchar_t>;

#undef DEFAULT_LOG_LEVEL

using default_logger_p = default_logger_t *const;
extern default_logger_p default_logger;
}