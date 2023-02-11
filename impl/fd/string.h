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
concept can_reserve = requires(T obj) { obj.reserve(2u); };

template <typename T, typename... Args>
concept can_append = requires(T val, Args... args) { val.append(std::forward<Args>(args)...); };

template <typename T, typename... Args>
concept can_push_back = requires(T val, Args... args) { val.push_back(std::forward<Args>(args)...); };

template <typename T, typename... Args>
concept can_insert = requires(T val, Args... args) { val.insert(val.end(), std::forward<Args>(args)...); };

template <typename T>
static constexpr auto _extract_size(const T& obj)
{
    if constexpr (std::is_class_v<T>)
        return std::pair(_begin(obj), _size(obj));
    else if constexpr (std::is_pointer_v<T>)
        return std::pair(obj, str_len(obj));
    else if constexpr (std::is_bounded_array_v<T>)
        return std::pair(std::begin(obj), *std::rbegin(obj) == '\0' ? std::size(obj) - 1 : std::size(obj));
    else
        return std::pair(obj, static_cast<size_t>(1));
}



template <typename Q, typename T, typename S>
static constexpr void _write_to(Q& dst, std::pair<T, S>& data)
{
    auto [src, size] = data;
    if (size == 0)
        return;

    if constexpr (iterator<Q>)
    {
        if constexpr (iterator<T>)
            copy(src, size, dst);
        else
            fill(dst, src, size);
        std::advance(dst, data.second);
    }
    else
    {
        if constexpr (iterator<T>)
            dst.insert(dst.end(), src, src + size);
        else
        {
            while (size-- != 0)
                dst.push_back(src);
        }
    }
}

template <bool Reserve, can_reserve T, typename... Args>
constexpr void write_string(T& buff, const Args&... args)
{
    if constexpr (Reserve && sizeof...(Args) > 1)
    {
        if (buff.empty())
        {
            return [&](auto... p) {
                const auto length = (p.second + ...);
                buff.reserve(length);
                (_write_to(buff, p), ...);
            }(_extract_size(args)...);
        }
    }

    [&](auto... p) {
        (_write_to(buff, p), ...);
    }(_extract_size(args)...);
}

template <typename T, typename... Args>
constexpr void write_string(T& buff, const Args&... args)
{
    write_string<can_reserve<T>>(buff, args...);
}

struct _make_string_helper
{
    template <typename T>
    static auto extract_value()
    {
        if constexpr (iterator<T>)
            return std::iter_value_t<T>();
        else if constexpr (native_iterable<T>)
            return std::iter_value_t<iter_t<T>>();
        else
            return T();
    }

    template <typename T, typename... Args>
    static void biggest_type_filter()
    {
        static_assert((std::same_as<std::remove_const_t<T>, Args> || ...));
    }

    template <typename Ret, typename T, typename... Args>
    static auto biggest_type()
    {
#ifdef _DEBUG
        biggest_type_filter<T, char, char8_t, uint8_t, int8_t, wchar_t, char16_t, char32_t>();
#endif
        if constexpr (std::is_void_v<Ret>)
        {
            if constexpr (sizeof...(Args) == 0 || (std::same_as<T, Args> && ...))
                return T();
            else
                return biggest_type<T, Args...>();
        }
        else
        {
            using result = std::conditional_t<sizeof(Ret) <= sizeof(T), Ret, T>;
            if constexpr (sizeof...(Args) != 0)
                return biggest_type<result, Args...>();
            else
                return result();
        }
    }
};

template <typename... Args>
constexpr auto make_string(const Args&... args)
{
    using h         = _make_string_helper;
    using char_type = decltype(h::biggest_type<void, decltype(h::extract_value<Args>())...>());
    basic_string<char_type> buff;
    write_string(buff, args...);
    return buff;
}

#pragma warning(pop)
} // namespace fd