module;

#include <xxh32.hpp>
#include <xxh64.hpp>

#include <array>
#include <bit>
#include <concepts>
#include <memory>

export module fd.hash;

constexpr size_t ct_strlen(const char* p)
{
#if 1
    return __builtin_strlen(p);
#else
    // to avoid string include
    size_t len = 0;
    do
    {
        ++len;
    }
    while (*++p != '\0');
    return len;
#endif
}

export namespace fd
{
    constexpr size_t _Hash_bytes(const char* input, const size_t len)
    {
        using xxh = std::conditional_t<sizeof(size_t) == 4, xxh32, xxh64>;
        return xxh::hash(input, len, 0);
    }

    template <typename T>
    constexpr size_t _Hash_bytes(const T* input, const size_t len) requires(!std::same_as<char, T>)
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

#ifdef FD_WORK_DIR
#define STR0(x)   #x
#define STR(x)    STR0(x)
#define SIZE_SKIP std::size(STR(FD_WORK_DIR)) - 1
#else
#define SIZE_SKIP 0
#endif

#if 1
    template <typename T, size_t S>
    constexpr size_t unique_hash(const T (&file_name)[S])
    {
        return _Hash_bytes(file_name + SIZE_SKIP, S - SIZE_SKIP - 1);
    }
#else
    constexpr size_t unique_hash(const std::source_location sl = std::source_location::current())
    {
        auto fname = sl.file_name() + SIZE_SKIP;
        return _Hash_bytes(fname, ct_strlen(fname));
    }
#endif

    template <typename T>
    struct hash;

    template <typename T>
    constexpr size_t _Hash_object(const T& obj)
    {
        hash<T> h;
        return h(obj);
    }
} // namespace fd
