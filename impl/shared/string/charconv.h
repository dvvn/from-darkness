#pragma once

#include "diagnostics/fatal.h"

#include <charconv>

namespace fd
{
template <size_t S, typename T>
constexpr std::from_chars_result from_chars(char const (&buff)[S], T& out)
{
    return std::from_chars(std::begin(buff), std::end(buff) - 1, out);
}

namespace detail
{
template <char... C>
struct from_char_cache
{
    static constexpr char buffer[]        = {C...};
    static constexpr bool null_terminated = *std::rbegin(buffer) == '\0';

    static constexpr char const* begin()
    {
        return std::begin(buffer);
    }

    static constexpr char const* end()
    {
        return std::end(buffer) - null_terminated;
    }

    template <typename T>
    static constexpr auto to(T& value, int base)
    {
        return std::from_chars(begin(), end(), value, base);
    }

    template <typename T>
    static constexpr T to(int base)
    {
        T tmp       = 0;
        auto result = to(tmp, base);
        if (result.ec != std::errc())
            unreachable();
        return tmp;
    }

    template <typename T, int Base>
    static constexpr T value = to<T>(Base);
};

template <size_t I>
constexpr size_t extract_index(std::in_place_index_t<I>)
{
    return I;
}
} // namespace detail

template <typename T, size_t I, char... C>
constexpr T from_chars()
{
    return detail::from_char_cache<C...>::template value<T, I>;
}

template <char... C>
constexpr void from_chars(auto& out, auto base_wrapped = std::in_place_index<10>)
{
    using type         = std::decay_t<decltype(out)>;
    constexpr int base = detail::extract_index(base_wrapped);

    out = detail::from_char_cache<C...>::template value<type, base>;
}

using std::from_chars;
}