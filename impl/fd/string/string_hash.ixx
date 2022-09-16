module;

#include <algorithm>

export module fd.string.hash;
export import fd.string;
export import fd.hash;

using namespace fd;

template <typename Chr, size_t Size>
struct trivial_chars_cache
{
    Chr arr[Size];

    constexpr trivial_chars_cache(const Chr* str_source)
    {
        std::copy_n(str_source, Size, arr);
    }

    constexpr const Chr* begin() const
    {
        return arr;
    }

    constexpr size_t size() const
    {
        return Size - 1;
    }
};

template <typename Chr, size_t Size>
trivial_chars_cache(const Chr (&arr)[Size]) -> trivial_chars_cache<Chr, Size>;

template <typename T>
class hashed_string_view : public basic_string_view<T>
{
    size_t hash_;

  public:
    using value_type = T;

    constexpr hashed_string_view(const T* ptr, const size_t size)
        : basic_string_view<T>(ptr, size)
    {
        hash_ = _Hash_bytes(ptr, size);
    }

    constexpr operator size_t const()
    {
        return hash_;
    }

    constexpr bool operator==(const hashed_string_view& other) const
    {
        return hash_ == other.hash_;
    }
};

template <typename T>
hashed_string_view(const T*, const size_t) -> hashed_string_view<T>;

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
        template <trivial_chars_cache Cache>
        consteval auto operator"" _hash() -> hashed_string_view<typename std::remove_const_t<decltype(Cache)>::value_type>
        {
            return { Cache.begin(), Cache.size() };
        }
    } // namespace literals
} // namespace fd
