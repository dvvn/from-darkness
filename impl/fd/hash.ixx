module;

#include <xxh32.hpp>
#include <xxh64.hpp>

#include <array>
#include <bit>
#include <concepts>
#include <memory>

export module fd.hash;

export namespace fd
{
    template <typename T>
    constexpr size_t _Hash_bytes(const T* input, const size_t len)
    {
        if constexpr (sizeof(T) == sizeof(char) && std::convertible_to<T, char>)
        {
            using xxh = std::conditional_t<sizeof(size_t) == sizeof(int32_t), xxh32, xxh64>;
            return xxh::hash(input, len, 0);
        }
        else
        {
            const auto bytes_count = sizeof(T) * len;

            if (!std::is_constant_evaluated())
            {
                const auto buff = reinterpret_cast<const char*>(input);
                return _Hash_bytes(buff, bytes_count);
            }
            else
            {
                using char_alloc = std::allocator<char>;

                // direct bit_cast like reinterpret_cast doesn't work
#if 0
                using buff_t = std::vector<char, char_alloc>;
                buff_t buff;
                buff.reserve(bytes_count);
                for (size_t i = 0; i < len; ++i)
                {
                    const auto tmp = std::bit_cast<std::array<char, sizeof(T)>>(input[i]);
                    for (const auto c : tmp)
                        buff.push_back(c);
                }
                return _Hash_bytes(buff.data(), buff.size());
#else
                char_alloc alloc;
                const auto buff = alloc.allocate(bytes_count);
                for (size_t i = 0; i < len; ++i)
                {
                    const auto tmp  = std::bit_cast<std::array<char, sizeof(T)>>(input[i]);
                    const auto desc = buff + i * tmp.size();
                    std::copy(tmp.begin(), tmp.end(), desc);
                }
                const auto result = _Hash_bytes(buff, bytes_count);
                alloc.deallocate(buff, bytes_count);
                return result;
#endif
            }
        }
    }

    template <typename T>
    struct hash
    {
        constexpr size_t operator()(const T* input, const size_t len) const
        {
            return _Hash_bytes(input, len);
        }
    };
} // namespace fd
