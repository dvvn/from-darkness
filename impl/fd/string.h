// ReSharper disable CppUserDefinedLiteralSuffixDoesNotStartWithUnderscore
#pragma once

#ifdef _DEBUG
#include <fd/exception.h>
#endif

#include <fd/algorithm.h>
#include <fd/views.h>

#include <array>
#include <iterator>
#include <string>

#pragma warning(push)
#pragma warning(disable : 4455)

namespace fd
{
using std::basic_string;
using std::basic_string_view;
using std::string;
using std::string_view;
using std::wstring;
using std::wstring_view;
#ifdef __cpp_lib_char8_t
using std::u8string;
using std::u8string_view;
#endif
using std::u16string;
using std::u16string_view;
using std::u32string;
using std::u32string_view;

template <typename C>
static constexpr bool _ptr_equal(const C* ptr1, const C* ptr2, const size_t count)
{
    return std::char_traits<C>::compare(ptr1, ptr2, count) == 0;
}

template <typename C>
static constexpr bool _ptr_equal_legacy(const C* myPtr, const C* unkPtr, const size_t count)
{
    if (std::is_constant_evaluated())
        return _ptr_equal(myPtr, unkPtr, count) && unkPtr[count] == static_cast<C>('\0');

    // unsafe (TESTING)
    return unkPtr[count] == static_cast<C>('\0') && _ptr_equal(myPtr, unkPtr, count);
}

template <class R, class L>
static constexpr bool _str_equal(const R& left, const L& right)
{
    const auto size = left.size();
    if constexpr (std::is_class_v<L>)
        return (size == right.size() && _ptr_equal(left.data(), right.data(), size));
    else if constexpr (std::is_bounded_array_v<L>)
        return size == std::size(right) - 1 && _ptr_equal(left.data(), right, size);
    else
        return (_ptr_equal_legacy(left.data(), right, size));
}

template <class T, class... Test>
concept other_than = (!std::same_as<T, Test> && ...);

template <class T, typename C>
concept not_string = other_than<T, basic_string<C>, basic_string_view<C>>;

// ReSharper disable CppInconsistentNaming

template <typename Test, typename C>
concept _same_str_type = requires(const Test& v) { static_cast<const C*>(std::data(v)); };

template <typename Test, typename C>
concept _only_same_str_type = _same_str_type<Test, C> && not_string<Test, C>;

// ReSharper restore CppInconsistentNaming

template <typename C, _same_str_type<C> Other>
constexpr bool operator==(const basic_string_view<C> left, const Other& right)
{
    return _str_equal(left, right);
}

template <typename C, _same_str_type<C> Other>
constexpr bool operator==(const basic_string<C>& left, const Other& right)
{
    return _str_equal(left, right);
}

template <typename C, _only_same_str_type<C> Other>
constexpr bool operator==(const Other& left, const basic_string_view<C> right)
{
    return right == left;
}

template <typename C, _only_same_str_type<C> Other>
constexpr bool operator==(const Other& left, const basic_string<C>& right)
{
    return right == left;
}

template <typename T>
constexpr size_t str_len(const T* str)
{
    return std::char_traits<T>::length(str);
}

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

//--------------

template <typename T>
concept can_iter_value = requires { typename std::iter_value_t<T>; };

template <typename T, bool = can_iter_value<T>>
struct extract_value;

template <typename T>
struct extract_value<T, false>
{
    using type = T;
};

template <typename T>
struct extract_value<T, true>
{
    using type = std::iter_value_t<T>;
};

template <typename T>
concept can_reserve = requires(T obj) { obj.reserve(2u); };

template <typename T, typename... Args>
concept can_append = requires(T val, Args... args) { val.append(std::forward<Args>(args)...); };

template <typename T>
static constexpr auto _extract_size(const T& obj)
{
    if constexpr (std::is_class_v<T>)
        return std::pair(forward_view_lazy(obj).begin(), obj.size());
    else if constexpr (std::is_pointer_v<T>)
        return std::pair(obj, str_len(obj));
    else if constexpr (std::is_bounded_array_v<T>)
        return std::pair(std::begin(obj), *std::rbegin(obj) == '\0' ? std::size(obj) - 1 : std::size(obj));
    else
        return std::pair(obj, static_cast<size_t>(1));
}

template <class Itr, typename T, typename S>
static constexpr void _append_to(Itr& buff, const std::pair<T, S>& obj)
{
    auto [src, size] = obj;
    if (size == 0)
        return;

    using itr_t            = std::remove_cvref_t<Itr>;
    constexpr auto canCopy = std::is_pointer_v<T> || std::is_class_v<T> /* std::input_iterator<itr_t> */;
    if constexpr (std::input_or_output_iterator<itr_t>)
    {
        if constexpr (canCopy)
            copy(src, size, buff);
        else
            fill(buff, size, src);
    }
    else
    {
        if constexpr (!canCopy)
            fill(std::back_insert_iterator(buff), size, src);
        else if constexpr (can_append<Itr&, T, T>)
            buff.append(src, src + size);
        else
            buff.insert(buff.end(), src, src + size);
    }
}

template <bool Reserve, typename... Args>
constexpr void write_string(can_reserve auto& buff, const Args&... args)
{
    if constexpr (Reserve && sizeof...(Args) > 1)
    {
        if (buff.empty())
        {
            return [&](auto... p) {
                const auto length = (p.second + ...);
                buff.reserve(length);
                (_append_to(buff, p), ...);
            }(_extract_size(args)...);
        }
    }

    [&](auto... p) {
        (_append_to(buff, p), ...);
    }(_extract_size(args)...);
}

template <typename T, typename... Args>
constexpr void write_string(T&& buff, const Args&... args)
{
    if constexpr (can_reserve<T&&>)
    {
        static_assert(!std::is_rvalue_reference_v<T&&>);
        return write_string<true>(buff, args...);
    }
    else
    {
        static_assert(std::forward_iterator<T>);
        (
            [&](auto p) {
                _append_to(buff, p);
                buff += p.second;
            }(_extract_size(args)),
            ...
        );
    }
}

template <typename T, typename... Args>
constexpr auto make_string(const T& arg1, const Args&... args)
{
    using char_type = typename extract_value<T>::type;
    basic_string<char_type> buff;
    write_string(buff, arg1, args...);
    return buff;
}

#if 0 // this code suck, wind a better way
// https://github.com/tcsullivan/constexpr-to-string

static constexpr string_view _Digits("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");

template <typename C, typename T>
static constexpr basic_string<C> _to_string(const T num, const uint8_t base)
{
#ifdef _DEBUG
    if (base <= 1 || base >= _Digits.size())
        abort();
#endif
    if (num == 0)
        return {};

    basic_string<C> buff;

    size_t len = (num > 0 ? 0 : 1);
    for (auto n = num; n; ++len, n /= base)
    {
    }
    buff.resize(len);

    auto ptr = buff.data() + len;
    for (auto n = num; n; n /= base)
        *--ptr = _Digits[(num < 0 ? -1 : 1) * (n % base)];
    if (num < 0)
        *--ptr = '-';

    return buff;
}

template <typename C>
static constexpr basic_string<C> _to_string(long double value, const bool trim, const uint8_t prec)
{
#pragma warning(disable : 4244)
    int64_t whole = value;
    value -= whole;
    for (size_t i = 0; i < prec; i++)
        value *= 10;
    int64_t frac = value;

    //---------

    size_t len = 1;
    if (whole <= 0)
        len++;
    for (auto n = whole; n; len++, n /= 10)
    {
    }
    if (frac == 0 || (whole == 0 && frac < 0))
        len++;
    for (auto n = frac; n; len++, n /= 10)
    {
    }

    basic_string<C> buff;
    buff.resize(len);
    auto ptr = buff.data() + len;

    const auto append = [&ptr](const auto num) {
        if (num == 0)
        {
            *--ptr = '0';
        }
        else
        {
            for (auto n = num; n != 0; n /= 10)
                *--ptr = (num < 0 ? -1 : 1) * (n % 10) + '0';
        }
    };

    append(frac);
    *--ptr = '.';
    append(whole);
    if (frac < 0 || whole < 0)
        *--ptr = '-';

    if (trim)
    {
        size_t offset = 0;
        for (auto itr = buff.rbegin(); itr != buff.rend(); ++itr)
        {
            if (*itr != '0')
            {
                if (offset > 0)
                    buff.resize(buff.size() - offset);
                break;
            }
            ++offset;
        }
    }
    return buff;
}

constexpr auto to_string(const std::integral auto num, const uint8_t base = 10)
{
    return _to_string<char>(num, base);
}

constexpr auto to_wstring(const std::integral auto num, const uint8_t base = 10)
{
    return _to_string<wchar_t>(num, base);
}

constexpr auto to_string(const std::floating_point auto num, const bool trim = false, const uint8_t prec = 5)
{
    return _to_string<char>(num, trim, prec);
}

template <typename T>
constexpr auto to_wstring(const std::floating_point auto num, const bool trim = false, const uint8_t prec = 5)
{
    return _to_string<wchar_t>(num, trim, prec);
}
#ifdef _DEBUG
static_assert(to_string(1234u) == "1234");
static_assert(to_string(-1234) == "-1234");
static_assert(to_string(1.234, false, 5) == "1.23400");
static_assert(to_string(1.234, true) == "1.234");
static_assert(to_string(-1.23456) == "-1.23456");
#endif

inline namespace literals
{
inline namespace string_literals
{
constexpr string operator"" s(const unsigned long long num)
{
    return to_string(num);
}

constexpr string operator"" s(const long double num)
{
    return to_string(num);
}
} // namespace string_literals
} // namespace literals
#endif

#pragma warning(pop)
} // namespace fd