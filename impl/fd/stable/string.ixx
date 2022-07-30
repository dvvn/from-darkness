module;

#include <fd/utility.h>

#include <string>

export module fd.string;
export import fd.hash;

#define _ALL_TYPES(_FN_, ...)      \
    _FN_(char, ##__VA_ARGS__);     \
    _FN_(wchar_t, ##__VA_ARGS__);  \
    _FN_(char8_t, ##__VA_ARGS__);  \
    _FN_(char16_t, ##__VA_ARGS__); \
    _FN_(char32_t, ##__VA_ARGS__);

#define PREFIX_char
#define PREFIX_wchar_t  w
#define PREFIX_char8_t  u8
#define PREFIX_char16_t u16
#define PREFIX_char32_t u32

#define _ADD_PREFIX0(_PREFIX_, _NAME_) _PREFIX_##_NAME_
#define _ADD_PREFIX1(_PREFIX_, _NAME_) _ADD_PREFIX0(_PREFIX_, _NAME_)
#define _ADD_PREFIX(_C_, _NAME_)       _ADD_PREFIX1(PREFIX_##_C_, _NAME_)

#define _EMPTY

#define _USING(_C_, _NAME_) using _ADD_PREFIX(_C_, _NAME_) = basic_##_NAME_<_C_>;

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

using std::basic_string;
using std::basic_string_view;

struct check_null_chr_t
{
};

constexpr check_null_chr_t check_null_chr;

template <typename C>
constexpr bool _Ptr_equal(const C* ptr1, const C* ptr2, const size_t count)
{
    return std::char_traits<C>::compare(ptr1, ptr2, count) == 0;
}

template <typename C>
constexpr bool _Ptr_equal(const C* my_ptr, const C* unk_ptr, const size_t count, const check_null_chr_t)
{
    return _Ptr_equal(my_ptr, unk_ptr, count) && unk_ptr[count] == static_cast<C>('\0');
}

template <class R, class L>
constexpr bool _Str_equal(const R& left, const L& right)
{
    return left.size() == right.size() && _Ptr_equal(left.data(), right.data(), left.size());
}

template <typename C>
constexpr bool operator==(const basic_string_view<C> left, const basic_string_view<C> right)
{
    return _Str_equal(left, right);
}

template <typename C>
constexpr bool operator==(const basic_string_view<C> left, const basic_string<C>& right)
{
    return _Str_equal(left, right);
}

template <typename C>
constexpr bool operator==(const basic_string<C>& left, const basic_string_view<C> right)
{
    return _Str_equal(left, right);
}

template <typename C>
constexpr bool operator==(const basic_string_view<C> left, const C* right)
{
    return _Ptr_equal(left.data(), right, left.size(), check_null_chr);
}

template <typename C>
constexpr bool operator==(const C* left, const basic_string_view<C> right)
{
    return right == left;
}

//---

template <typename C>
constexpr bool operator==(const basic_string<C>& left, const basic_string<C>& right)
{
    return left.size() == right.size() && _Ptr_equal(left.data(), right.data(), left.size());
}

template <typename C>
constexpr bool operator==(const basic_string<C>& left, const C* right)
{
    return _Ptr_equal(left.data(), right, left.size(), check_null_chr);
}

template <typename C>
constexpr bool operator==(const C* left, const basic_string<C>& right)
{
    return right == left;
}

export namespace fd
{
    template <typename T>
    constexpr size_t str_len(const T* ptr)
    {
        return std::char_traits<T>::length(ptr);
    }

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
        consteval size_t operator"" _hash()
        {
            return _Hash_bytes(Cache.begin(), Cache.size());
        }

        static_assert("test"_hash == u8"test"_hash);
        static_assert(u"test"_hash == "t\0e\0s\0t\0"_hash);
        static_assert(U"test"_hash == u"t\0e\0s\0t\0"_hash);
        static_assert(U"ab"_hash == "a\0\0\0b\0\0\0"_hash);
    } // namespace literals

    _ALL_TYPES(_USING, string);
    _ALL_TYPES(_USING, string_view);

    using ::basic_string;
    using ::basic_string_view;

    /* _ALL_TYPES(_USING, hashed_string);
    _ALL_TYPES(_USING, hashed_string_view);

    using ::basic_hashed_string;
    using ::basic_hashed_string_view; */

    using ::operator==;

} // namespace fd
