#pragma once

#include <xxh32.hpp>
#include <xxh64.hpp>

#include <algorithm>
#include <array>
#include <concepts>
#include <memory>
#ifdef __cpp_lib_bit_cast
#include <bit>
#else
#include <vector>
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

            // ReSharper disable CppUnreachableCode
            if (!std::is_constant_evaluated())
            {
                const auto buff = reinterpret_cast<const char*>(input);
                return hash_bytes(buff, bytesCount);
            }
            // ReSharper restore CppUnreachableCode

            using char_alloc = std::allocator<char>;

            // direct bit_cast like reinterpret_cast doesn't work
#ifdef __cpp_lib_bit_cast
            char_alloc alloc;
            const auto buff = alloc.allocate(bytesCount);
            for (size_t i = 0; i < len; ++i)
            {
                const auto tmp  = std::bit_cast<std::array<char, sizeof(T)>>(input[i]);
                const auto desc = buff + i * tmp.size();
                std::ranges::copy(tmp, desc);
            }
            const auto result = hash_bytes(buff, bytesCount);
            alloc.deallocate(buff, bytesCount);
            return result;
#else
            using buff_t = std::vector<char, char_alloc>;
            buff_t buff;
            buff.reserve(bytes_count);
            for (size_t i = 0; i < len; ++i)
            {
                const auto tmp = std::bit_cast<std::array<char, sizeof(T)>>(input[i]);
                for (const auto c : tmp)
                    buff.push_back(c);
            }
            return hash_bytes(buff.data(), buff.size());
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
