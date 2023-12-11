#pragma once
#include <type_traits>

namespace fd
{
template <typename T>
using type_identity = std::type_identity<T>;

template <typename T>
inline constexpr type_identity<T> type_identity_v;
} // namespace fd