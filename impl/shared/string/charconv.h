#pragma once

#include <charconv>

namespace fd
{
using std::from_chars;

template <size_t S, typename T>
constexpr std::from_chars_result from_chars(char const (&buff)[S], T &out)
{
    return from_chars(std::begin(buff), std::end(buff) - 1, out);
}

namespace detail
{
template <char... C>
struct from_char_buffer
{
    static constexpr char buffer[]        = {C...};
    static constexpr bool null_terminated = *std::prev(std::end(buffer)) == '\0';

    static constexpr char const *begin()
    {
        return std::begin(buffer);
    }

    static constexpr char const *end()
    {
        return std::end(buffer) - null_terminated;
    }
};

template <typename T, int Base, char... C>
struct from_char_cache
{
    static constexpr T value = [] {
        using buff  = from_char_buffer<C...>;
        T tmp       = 0;
        auto result = from_chars(buff::begin(), buff::end(), tmp, Base);
        if (result.ec != std::errc())
            std::unreachable();
        return tmp;
    }();
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
    return detail::from_char_cache<T, I, C...>::value;
}

template <char... C>
constexpr std::from_chars_result from_chars(auto &out, auto base_wrapped = std::in_place_index<10>)
{
    using type         = std::remove_reference_t<decltype(out)>;
    constexpr int base = extract_index(base_wrapped);

    out = detail::from_char_cache<type, base, C...>::value;
    return {};
}
}