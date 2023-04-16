#pragma once

#include <fmt/format.h>

namespace fd
{
template <typename... Args>
auto to_string(fmt::format_string<Args...> fmt, Args &&...args)
{
    return fmt::vformat(fmt, fmt::make_format_args(args...));
}
}