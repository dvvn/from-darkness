#pragma once

#include "type_traits.h"

#include <array>

namespace fd
{
template <typename T, size_t Length>
using array = std::array<T, Length>;

template <typename T, size_t Length>
constexpr auto size(array<T, Length> const&) -> integral_constant<detail::small_type<size_t, Length>, Length>
{
    return {};
}
} // namespace fd