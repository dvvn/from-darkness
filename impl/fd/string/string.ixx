module;

#include <fd/utility.h>

#include <tuple>

#define explicit
#include <string>

export module fd.string;
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

using std::basic_string;
using std::basic_string_view;

struct check_null_chr_t
{
} constexpr check_null_chr;

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
    // return _Str_equal(left, right);
    const auto size = left.size();
    if (size != right.size())
        return false;

    const auto ld = left.data();
    const auto rd = right.data();

    if (ld == rd)
        return true;

    return _Ptr_equal(ld, rd, size);
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
    constexpr size_t str_len(const T* str)
    {
        return std::char_traits<T>::length(str);
    }

    _ALL_TYPES(_USING, string);
    _ALL_TYPES(_USING, string_view);

    using ::basic_string;
    using ::basic_string_view;

    using ::operator==;

    inline namespace literals
    {
        inline namespace string_view_literals
        {
            using std::string_view_literals::operator""sv;
        }

        inline namespace string_literals
        {
            using std::string_literals::operator""s;
        }
    } // namespace literals

} // namespace fd
