#pragma once

#include <type_traits>

namespace fd
{
template <typename P, size_t Count = 1, bool = std::is_pointer_v<P>>
struct remove_pointer;

template <typename P>
struct remove_pointer<P, 0, true> : std::type_identity<P>
{
};

template <typename P, size_t Count>
struct remove_pointer<P, Count, false> : std::type_identity<P>
{
};

template <typename P, size_t Count>
struct remove_pointer<P* const, Count, true> : remove_pointer<P, Count - 1>
{
};

template <typename P, size_t Count>
struct remove_pointer<P*, Count, true> : remove_pointer<P, Count - 1>
{
};

template <typename P, size_t Count = 1>
using remove_pointer_t = typename remove_pointer<P, Count>::type;
} // namespace fd