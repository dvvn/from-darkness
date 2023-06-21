#pragma once

// #include <type_traits>
#include <cstddef>

namespace fd
{
template <size_t N>
struct named_arg
{
    char buff[N];

    char const *begin() const
    {
        return buff;
    }

    char const *end() const
    {
        return buff + N - 1;
    }

    constexpr size_t length() const
    {
        return N - 1;
    }

    constexpr named_arg(char const (&arg_name)[N])
    {
        if (std::is_constant_evaluated())
        {
            for (size_t i = 0; i != N; ++i)
                buff[i] = arg_name[i];
        }
    }
};
} // namespace fd