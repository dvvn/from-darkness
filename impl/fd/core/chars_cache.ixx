module;

#include <string_view>

export module fd.chars_cache;

template <typename Chr, size_t Size>
struct simple_chars_cache
{
    Chr _Data[Size];

    using value_type = Chr;
    using pointer    = const Chr*;

    constexpr simple_chars_cache(pointer str_source, const size_t string_size = Size)
    {
        std::copy_n(str_source, string_size, _Data);
        if (string_size < Size)
            std::fill(_Data + string_size, _Data + Size, static_cast<Chr>(0));
    }

    constexpr size_t size() const
    {
        return Size - 1;
    }

    constexpr pointer begin() const
    {
        return _Data;
    }

    constexpr pointer end() const
    {
        return begin() + size();
    }

    constexpr std::basic_string_view<Chr> view() const
    {
        return { begin(), size() };
    }

    constexpr operator std::basic_string_view<Chr>() const
    {
        return view();
    }
};

template <typename Chr, size_t Size>
simple_chars_cache(const Chr (&arr)[Size]) -> simple_chars_cache<Chr, Size>;

/* #ifdef _DEBUG */ #if 1

    template <typename Chr, size_t Size>
    struct chars_cache : simple_chars_cache <Chr, Size>
{
    using simple_chars_cache<Chr, Size>::simple_chars_cache;
};

template <typename Chr, size_t Size>
chars_cache(const Chr (&arr)[Size]) -> chars_cache<Chr, Size>;

#else
#error xoring not implemented
#endif

/* template <typename Chr, size_t Size, std::convertible_to<std::basic_string_view<Chr>> Str>
constexpr auto operator==(const simple_chars_cache<Chr, Size>& left, const Str& right)
{
    return left.view() == right;
}

template <typename Chr, size_t Size, std::convertible_to<std::basic_string_view<Chr>> Str>
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
    using ::chars_cache;
    using ::simple_chars_cache;

    inline namespace literals
    {
        using ::operator"" _cch;
    }

    // using ::operator==;

} // namespace fd
