#pragma once

#include <cstddef>

namespace fd
{
template <size_t S>
constexpr size_t strlen(char const (&)[S])
{
    return S - 1;
}

constexpr bool islower(char c)
{
    return c >= 'a' && c <= 'z';
}

constexpr bool isupper(char c)
{
    return c >= 'A' && c <= 'Z';
}

constexpr bool isdigit(char c)
{
    return c >= '0' && c <= '9';
}
} // namespace fd
