#pragma once

#include <cstddef>

namespace fd
{
template <size_t N>
struct named_arg
{
    [[no_unique_address]] //
    struct
    {
    } dummy;

    constexpr named_arg(char const (&)[N])
    {
    }
};
} // namespace fd
