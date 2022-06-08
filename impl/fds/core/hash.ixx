module;

//#include <constexpr-xxh3.h>
#include <xxh32.hpp>
#include <xxh64.hpp>

#include <array>
#include <bit>
#include <string>
//#include <vector>

export module fds.hash;
import fds.chars_cache;

template <typename T>
struct hash
{
    template <class Alloc = std::allocator<char>>
    constexpr auto operator()(const T* input, const size_t len, const Alloc& = {}) const
    {
#if 0
        using namespace constexpr_xxh3;
        if constexpr (ByteType<T>)
        {

            // return XXH3_64bits_const(input, len);
            return XXH3_64bits_internal(input, len, 0, kSecret, sizeof(kSecret), [](const auto input, const auto len, const auto /*seed*/, const auto secret, const auto secret_size) {
                return hashLong_64b_internal(input, len, secret, secret_size);
            });
        }
        else
#endif

        using xxh = std::conditional_t<std::same_as<size_t, uint32_t>, xxh32, xxh64>;

        if constexpr (std::convertible_to<const T*, const char*>)
        {
            return xxh::hash(input, len, 0);
        }
        else
        {
            constexpr hash<char> char_hash;
            const auto bytes_count = sizeof(T) * len;

            if (!std::is_constant_evaluated())
            {
                const auto src = reinterpret_cast<const char*>(input);
                return char_hash(src, bytes_count);
            }
            else
            {
                using char_alloc = typename std::allocator_traits<Alloc>::template rebind_alloc<char>;

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
                return char_hash(buff.data(), buff.size());
#else
                char_alloc alloc;
                const auto buff = alloc.allocate(bytes_count);
                for (size_t i = 0; i < len; ++i)
                {
                    const auto tmp  = std::bit_cast<std::array<char, sizeof(T)>>(input[i]);
                    const auto desc = buff + i * tmp.size();
                    std::copy(tmp.begin(), tmp.end(), desc);
                }
                const auto result = char_hash(buff, bytes_count);
                alloc.deallocate(buff, bytes_count);
                return result;
#endif
            }
        }
    };
};

template <typename C, class Tr>
struct hash<std::basic_string_view<C, Tr>> : protected hash<C>
{
    constexpr auto operator()(const std::basic_string_view<C, Tr> str) const
    {
        return hash<C>::operator()(str.data(), str.size());
    }
};

template <typename C, class Tr, class Alloc>
struct hash<std::basic_string<C, Tr, Alloc>> : hash<std::basic_string_view<C, Tr>>
{
    constexpr auto operator()(const std::basic_string<C, Tr>& str) const
    {
        return hash<C>::operator()(str.data(), str.size(), Alloc());
    }
};

template <typename C, class Tr = std::char_traits<C>>
constexpr auto _Hash_strv(const std::basic_string_view<C, Tr> str)
{
    const hash<std::basic_string_view<C, Tr>> h;
    return h(str);
}

template <fds::simple_chars_cache Cache>
consteval auto operator"" _hash()
{
    return _Hash_strv(Cache.view());
}

static_assert("test"_hash == u8"test"_hash);
static_assert(u"test"_hash == "t\0e\0s\0t\0"_hash);
static_assert(U"test"_hash == u"t\0e\0s\0t\0"_hash);
static_assert(U"ab"_hash == "a\0\0\0b\0\0\0"_hash);

template <typename C, size_t S>
consteval size_t calc_hash(const C (&str)[S])
{
    return _Hash_strv<C>({str, str + S - 1});
}

static_assert("test"_hash == calc_hash("test"));

export namespace fds
{
    using ::calc_hash;
    using ::hash;

    inline namespace literals
    {
        using ::operator"" _hash;
    }
} // namespace fds
