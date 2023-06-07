#pragma once

#include <concepts>

namespace fd
{
template <typename From, typename To>
concept magic_convertible = sizeof(From) == sizeof(To) && std::is_trivial_v<From> && std::is_trivial_v<To>;

template <typename From, typename To>
class magic_cast;

template <typename T>
constexpr bool is_void_pointer_v = false;
template <>
constexpr bool is_void_pointer_v<void *> = true;
template <typename T>
constexpr bool is_void_pointer_v<T **> = is_void_pointer_v<T *>;
template <typename T>
constexpr bool is_void_pointer_v<T ***> = is_void_pointer_v<T *>;

template <typename To, typename From>
[[deprecated]]
To magic_cast_simple(From from) noexcept
{
    using wrapper = magic_cast<From, To>;
    if constexpr (std::destructible<wrapper>)
    {
        return wrapper(from);
    }
    else if constexpr (std::convertible_to<From, To> && (is_void_pointer_v<From> || is_void_pointer_v<To>))
    {
        return static_cast<To>(from);
    }
    else
    {
        static_assert(magic_convertible<From, To>);

        union
        {
            From tmp;
            To result;
        };

        tmp = from;
        return result;
    }
}

struct auto_cast_tag final
{
    auto_cast_tag() = delete;
};

template <typename From, typename To>
struct auto_cast_resolver;

}