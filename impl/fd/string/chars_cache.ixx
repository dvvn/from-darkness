module;

#include <algorithm>

export module fd.chars_cache;
export import fd.string;

#if 0

template <typename T>
class append_helper
{
    T pos_;

  public:
    constexpr append_helper(const T pos)
        : pos_(pos)
    {
    }

    template <typename Itr>
    constexpr append_helper append(const Itr from, const Itr to) const
    {
        return std::copy(from, to, pos_);
    }

    template <typename Itr>
    constexpr append_helper append(const Itr from, const std::iter_difference_t<Itr> count) const
    {
        return std::copy_n(from, count, pos_);
    }

    constexpr append_helper append(const std::iter_value_t<T> val) const
    {
        *pos_ = val;
        return pos_ + 1;
    }

#ifdef __cpp_lib_ranges
    template <std::ranges::range R>
    constexpr append_helper append(const R& rng) const
    {
        return std::ranges::copy(rng, pos_).out;
    }
#endif
};

#endif

template <typename Chr, size_t Size>
struct simple_chars_cache
{
    Chr _Data[Size];

    using value_type    = Chr;
    using pointer       = Chr*;
    using const_pointer = const Chr*;

    constexpr simple_chars_cache()
        : _Data()
    {
#ifndef _DEBUG
        _Data[Size - 1] = 0;
#endif
    }

    constexpr simple_chars_cache(const_pointer str_source, const size_t string_size = Size)
    {
        // assign(str_source, string_size);
        std::copy_n(str_source, string_size, _Data);
        std::fill(_Data + string_size, _Data + Size, static_cast<Chr>(0));
        //_Data[string_size] = 0;
    }

#if 0
    constexpr void assign(const_pointer str_source, const size_t string_size = Size)
    {
        std::copy_n(str_source, string_size, _Data);
        std::fill(_Data + string_size, _Data + Size, static_cast<Chr>(0));
        //_Data[string_size] = 0;
    }

    template <typename... Args>
    constexpr auto append(Args&&... args)
    {
        return append_helper(_Data).append(args...);
    }
#endif

    constexpr const_pointer data() const
    {
        return _Data;
    }

    constexpr const_pointer begin() const
    {
        return data();
    }

    constexpr const_pointer end() const
    {
        return begin() + size();
    }

    constexpr pointer data()
    {
        return _Data;
    }

    constexpr pointer begin()
    {
        return data();
    }

    constexpr pointer end()
    {
        return begin() + size();
    }

    constexpr size_t size() const
    {
        return Size - 1;
    }

    using _View = fd::basic_string_view<Chr>;

    constexpr _View view() const
    {
        return { begin(), size() };
    }

    constexpr operator _View() const
    {
        return view();
    }
};

template <typename Chr, size_t Size>
simple_chars_cache(const Chr (&arr)[Size]) -> simple_chars_cache<Chr, Size>;

#if 1
template <typename Chr, size_t Size>
struct chars_cache : simple_chars_cache<Chr, Size>
{
    using simple_chars_cache<Chr, Size>::simple_chars_cache;
};

template <typename Chr, size_t Size>
chars_cache(const Chr (&arr)[Size]) -> chars_cache<Chr, Size>;

#else
#error xoring not implemented
#endif

/* template <typename Chr, size_t Size, std::convertible_to<fd::basic_string_view<Chr>> Str>
constexpr auto operator==(const simple_chars_cache<Chr, Size>& left, const Str& right)
{
    return left.view() == right;
}

template <typename Chr, size_t Size, std::convertible_to<fd::basic_string_view<Chr>> Str>
constexpr auto operator==(const Str& left, const simple_chars_cache<Chr, Size>& right)
{
    return right == left;
} */

template <chars_cache Cache>
consteval auto& operator"" _cch()
{
    return Cache;
}

export namespace fd
{
    template <chars_cache C>
    constexpr auto& chars_cache_buff = C;

    template <simple_chars_cache C>
    constexpr auto& simple_chars_cache_buff = C;

#if 0

    template <typename C, size_t Size>
    struct hash<chars_cache<C, Size>>
    {
        constexpr size_t operator()(const chars_cache<C, Size>& str) const
        {
#if 1
            return _Hash_bytes(str.data(), str.size());
#else
#error xoring not implemented
#endif
        }
    };

    template <typename C, size_t Size>
    struct hash<simple_chars_cache<C, Size>>
    {
        constexpr size_t operator()(const simple_chars_cache<C, Size>& str) const
        {
            return _Hash_bytes(str.data(), str.size());
        }
    };

#endif

    using ::chars_cache;
    using ::simple_chars_cache;

    inline namespace literals
    {
        using ::operator"" _cch;
    }

    // using ::operator==;

} // namespace fd
