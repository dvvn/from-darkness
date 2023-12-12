#pragma once

#include <concepts>

namespace fd
{
namespace detail
{
template <typename T>
constexpr auto complete_check_helper()
{
    return sizeof(T);
}
} // namespace detail

template <class T>
concept complete = requires { sizeof(T); } && static_cast<bool>(detail::complete_check_helper<T>());
} // namespace fd