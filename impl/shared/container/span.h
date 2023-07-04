#pragma once

#include "internal/wrapper.h"

#include <span>

namespace fd
{
template <typename T>
FD_WRAP_TOOL(span, std::span<T, std::dynamic_extent>);

#if 0
template <typename... T>
span(T &&...args) -> span<typename decltype(std::span(static_cast<T &&>(args)...))::element_type>;
#else
template <std::contiguous_iterator It, class End>
span(It, End) -> span<std::remove_reference_t<std::iter_reference_t<It>>>;

template <std::ranges::contiguous_range Rng>
span(Rng &&) -> span<std::remove_reference_t<std::ranges::range_reference_t<Rng>>>;
#endif
}