#pragma once

#include <fmt/xchar.h>

namespace fd
{
template <typename... Args>
auto to_string(fmt::wformat_string<Args...> fmt, Args &&...args)
{
    return fmt::vformat(fmt, fmt::make_wformat_args(args...));
}
}