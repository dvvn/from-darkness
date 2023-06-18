#pragma once

// #include <type_traits>
#include <cstddef>

namespace fd
{
template <size_t N>
struct named_arg
{
    // char buff[N];

    constexpr named_arg(char const (&arg_name)[N])
    {
        /*if (!std::is_constant_evaluated())
            return;

        for (size_t i = 0; i != N; ++i)
            buff[i] = arg_name[i];*/
    }
};
} // namespace fd