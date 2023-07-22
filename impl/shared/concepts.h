#pragma once
#include <concepts>

namespace fd
{
template <class T>
concept complete = std::destructible<decltype(sizeof(T))>;
} // namespace fd