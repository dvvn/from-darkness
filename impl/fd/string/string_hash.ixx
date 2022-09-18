module;

#include <cstdint>

export module fd.string.hash;
export import fd.chars_cache;
export import fd.hash;

export namespace fd
{
    template <typename C>
    struct hash<basic_string_view<C>>
    {
        constexpr size_t operator()(const basic_string_view<C> str) const
        {
            return _Hash_bytes(str.data(), str.size());
        }
    };

    template <typename C>
    struct hash<basic_string<C>> : hash<basic_string_view<C>>
    {
        constexpr size_t operator()(const basic_string<C>& str) const
        {
            return _Hash_bytes(str.data(), str.size());
        }
    };

    inline namespace literals
    {
        template <chars_cache Cache>
        consteval size_t operator"" _hash()
        {
            return _Hash_bytes(Cache.data(), Cache.size());
        }
    } // namespace literals

#if 0
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
        return _Hash_bytes(fname, str_len(fname));
    }
#endif

#endif

} // namespace fd
