#pragma once
#include "tier0/core.h"

#include <concepts>

namespace FD_TIER(1)
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
} // namespace FD_TIER(1)