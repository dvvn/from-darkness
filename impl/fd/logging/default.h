#pragma once

#include <fd/logging/logger.h>

namespace fd
{
template <log_level Level, typename... C>
struct default_logger_for : logger<C, Level>...
{
    using logger<C, Level>::write...;
};

using default_logger_t = default_logger_for<
#ifdef _DEBUG
    log_level::debug,
#else
    log_level::err,
#endif
    char,
    wchar_t>;
using default_logger_p = default_logger_t *const;

extern default_logger_p default_logger;
}