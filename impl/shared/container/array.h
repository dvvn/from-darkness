#pragma once

#include "type_traits/integral_constant.h"
#include "type_traits/small_type.h"

#include <array>

namespace fd
{
template <typename T, size_t Length>
using array = std::array<T, Length>;

template <typename T, size_t Length>
constexpr auto size(array<T, Length> const&) -> integral_constant<small_type<size_t, Length>, Length>
{
    return {};
}

template <typename T, size_t Length>
constexpr auto size(T const (&)[Length]) -> integral_constant<small_type<size_t, Length>, Length>
{
    return {};
}
} // namespace fd