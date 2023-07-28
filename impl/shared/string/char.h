#pragma once

#include <cstddef>

namespace fd
{
template <size_t S>
constexpr size_t strlen(char const (&)[S])
{
    return S - 1;
}

template <size_t S>
constexpr size_t strlen(wchar_t const (&)[S])
{
    return S - 1;
}

namespace detail
{
constexpr bool is_betweet_two(char const c, char const left, char const rigtht)
{
    return c >= left && c <= rigtht;
}
} // namespace detail

constexpr bool islower(char const c)
{
    return detail::is_betweet_two(c, 'a', 'z');
}

constexpr bool isupper(char const c)
{
    return detail::is_betweet_two(c, 'A', 'Z');
}

constexpr bool isdigit(char const c)
{
    return detail::is_betweet_two(c, '0', '9');
}

constexpr bool isxdigit(char const c)
{
    return detail::is_betweet_two(c, '0', '9') || //
           detail::is_betweet_two(c, 'a', 'f') || //
           detail::is_betweet_two(c, 'A', 'F');
}

} // namespace fd