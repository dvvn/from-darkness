module;

//#include <constexpr-xxh3.h>
#include <xxh32.hpp>
#include <xxh64.hpp>

#include <array>
#include <bit>
#include <source_location>
#include <string>
//#include <vector>

export module fd.hash;

constexpr size_t _Hash_bytes(const char* input, const size_t len)
{
    using xxh = std::conditional_t<std::same_as<size_t, uint32_t>, xxh32, xxh64>;
    return xxh::hash(input, len, 0);
}

template <class Alloc>
constexpr size_t _Hash_bytes(const char* input, const size_t len, const Alloc&)
{
    return _Hash_bytes(input, len);
}

template <typename T, class Alloc = std::allocator<T>>
constexpr size_t _Hash_bytes(const T* input, const size_t len, const Alloc& = {}) requires(sizeof(T) > 1)
{
    const auto bytes_count = sizeof(T) * len;

    if (!std::is_constant_evaluated())
    {
        const auto buff = reinterpret_cast<const char*>(input);
        return _Hash_bytes(buff, bytes_count);
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

template <typename T>
struct hash
{
    template <class Alloc>
    constexpr size_t operator()(const T* input, const size_t len, const Alloc& a) const
    {
        return _Hash_bytes(input, len, a);
    };
};

template <typename C, class Tr>
struct hash<std::basic_string_view<C, Tr>> : protected hash<C>
{
    constexpr size_t operator()(const std::basic_string_view<C, Tr> str) const
    {
        return _Hash_bytes(str.data(), str.size());
    }
};

template <typename C, class Tr, class Alloc>
struct hash<std::basic_string<C, Tr, Alloc>> : hash<std::basic_string_view<C, Tr>>
{
    constexpr size_t operator()(const std::basic_string<C, Tr>& str) const
    {
        return _Hash_bytes(str.data(), str.size(), str.get_allocator());
    }
};

template <typename Chr, size_t Size>
struct trivial_chars_cache
{
    Chr arr[Size];

    constexpr trivial_chars_cache(const Chr* str_source, const size_t string_size = Size)
    {
        std::copy_n(str_source, string_size, arr);
    }
};

template <typename Chr, size_t Size>
trivial_chars_cache(const Chr (&arr)[Size]) -> trivial_chars_cache<Chr, Size>;

template <trivial_chars_cache Cache>
consteval size_t operator"" _hash()
{
    return _Hash_bytes(Cache.arr, std::size(Cache.arr) - 1);
}

static_assert("test"_hash == u8"test"_hash);
static_assert(u"test"_hash == "t\0e\0s\0t\0"_hash);
static_assert(U"test"_hash == u"t\0e\0s\0t\0"_hash);
static_assert(U"ab"_hash == "a\0\0\0b\0\0\0"_hash);

template <typename C, size_t S>
constexpr size_t calc_hash(const C (&str)[S])
{
    return _Hash_bytes(str, S - 1);
}

static_assert("test"_hash == calc_hash("test"));

#define STR0(x) #x
#define STR(x)  STR0(x)

constexpr size_t unique_hash(const std::source_location sl = std::source_location::current())
{
    auto fname = sl.file_name();
#ifdef FD_WORK_DIR
    fname += std::size(STR(FD_WORK_DIR)) - 1;
#endif
    return _Hash_bytes(fname, std::char_traits<char>::length(fname));
}

export namespace fd
{
    using ::calc_hash;
    using ::hash;
    using ::unique_hash;

    inline namespace literals
    {
        using ::operator"" _hash;
    }
} // namespace fd
