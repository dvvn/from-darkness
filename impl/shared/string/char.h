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

inline constexpr struct
{
    constexpr bool operator()(char const c) const
    {
        return detail::is_betweet_two(c, 'a', 'z');
    }

    bool operator()(wchar_t) const = delete;
} islower;

inline constexpr struct
{
    constexpr bool operator()(char const c) const
    {
        return detail::is_betweet_two(c, 'A', 'Z');
    }

    bool operator()(wchar_t) const = delete;
} isupper;

inline constexpr struct
{
    constexpr bool operator()(char const c) const
    {
        return detail::is_betweet_two(c, '0', '9');
    }

    bool operator()(wchar_t) const = delete;
} isdigit;

inline constexpr struct
{
    constexpr bool operator()(char const c) const
    {
        return detail::is_betweet_two(c, '0', '9') || //
               detail::is_betweet_two(c, 'a', 'f') || //
               detail::is_betweet_two(c, 'A', 'F');
    }

    bool operator()(wchar_t) const = delete;
} isxdigit;
} // namespace fd