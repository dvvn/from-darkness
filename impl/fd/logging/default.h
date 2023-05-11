#pragma once

#include "logger.h"

namespace fd
{
#ifdef FD_DEFAULT_LOG_LEVEL
#define DEFAULT_LOG_LEVEL FD_DEFAULT_LOG_LEVEL
#else
#ifdef _DEBUG
#define DEFAULT_LOG_LEVEL debug
#else
#define DEFAULT_LOG_LEVEL err
#endif
#endif

using default_logger = logger_same_level<log_level::DEFAULT_LOG_LEVEL, char, wchar_t>;

#undef DEFAULT_LOG_LEVEL

extern default_logger *get_default_logger();
}