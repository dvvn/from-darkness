#pragma once

#include "type_traits.h"

#include <span>

namespace fd
{
template <typename T>
using span = std::span<T>;

template <typename T, size_t Length>
requires(Length != std::dynamic_extent)
constexpr auto size(std::span<T, Length> const&) -> integral_constant<detail::small_type<size_t, Length>, Length>
{
    return {};
}
} // namespace fd