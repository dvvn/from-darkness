#pragma once

#include <fd/algorithm.h>
#include <fd/exception.h>

#include <xxh32.hpp>
#include <xxh64.hpp>

#include <array>
#include <concepts>
#include <memory>
#ifdef __cpp_lib_bit_cast
#include <bit>
#endif

namespace fd
{
template <typename T>
constexpr size_t hash_bytes(const T* input, const size_t len)
{
    if constexpr (sizeof(T) == sizeof(char) && std::convertible_to<T, char>)
    {
        using xxh = std::conditional_t<sizeof(size_t) == sizeof(int32_t), xxh32, xxh64>;
        return xxh::hash(input, len, 0);
    }
    else
    {
        const auto bytesCount = sizeof(T) * len;

        if (!std::is_constant_evaluated())
        {
            const auto buff = reinterpret_cast<const char*>(input);
            return hash_bytes(buff, bytesCount);
        }

#ifndef __cpp_lib_bit_cast
        abort();
#else
#ifdef __cpp_lib_constexpr_dynamic_alloc
        const auto buff = new char[bytesCount];
#else
        if (bytesCount >= 2048)
        {
            terminate();
            return 0;
        }
        char buff[2048];
#endif
        for (size_t i = 0; i < len; ++i)
        {
            const auto tmp  = std::bit_cast<std::array<char, sizeof(T)>>(input[i]);
            const auto desc = buff + i * tmp.size();
            copy(tmp, desc);
        }
        const auto result = hash_bytes(buff, bytesCount);
#ifdef __cpp_lib_constexpr_dynamic_alloc
        delete buff[];
#endif
        return result;
#endif
    }
}

template <typename T>
struct hash
{
    constexpr size_t operator()(const T* input, const size_t len) const
    {
        return hash_bytes(input, len);
    }
};
} // namespace fd