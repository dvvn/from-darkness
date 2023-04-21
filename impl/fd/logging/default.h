#pragma once

#include <fd/logging/logger.h>

namespace fd
{
#ifdef _DEBUG
#define DEFAULT_LOG_LEVEL log_level::debug
#else
#define DEFAULT_LOG_LEVEL log_level::err
#endif

struct default_logger_t : logger<char, DEFAULT_LOG_LEVEL>, logger<wchar_t, DEFAULT_LOG_LEVEL>
{
    static constexpr log_level level()
    {
        return DEFAULT_LOG_LEVEL;
    }
};

#undef DEFAULT_LOG_LEVEL

template <template <typename C> class T>
struct base_for_default_logger : default_logger_t, T<char>, T<wchar_t>
{
};

using default_logger_p = default_logger_t *const;
extern default_logger_p default_logger;
}