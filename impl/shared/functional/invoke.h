#pragma once

#include <functional>

namespace fd
{
template <typename Fn, typename... T>
constexpr decltype(auto) invoke(Fn* callable, T&&... args)
{
    return std::invoke(*callable, std::forward<T>(args)...);
}
}