#pragma once

#include "tier1/container/array.h"

#include <algorithm>
#include <cassert>
#include <concepts>

namespace FD_TIER(1)
{
template <size_t S>
constexpr size_t strlen(char const (&str)[S]) noexcept
{
    assert(*std::rbegin(str) == '\0');
    return S - 1;
}

// template <size_t S>
// constexpr size_t strlen(wchar_t const (&)[S])
//{
//     return S - 1;
// }

template <typename T>
constexpr bool is_betweet_two(T const c, T const left, T const rigtht)
{
    return c >= left && c <= rigtht;
}

inline constexpr array<char, UCHAR_MAX + 1> default_char_table = [] {
    array<char, UCHAR_MAX + 1> arr{};
#if 0
    auto ptr = arr.data();
    for (auto c = CHAR_MIN; c != CHAR_MAX; ++c)
        *ptr++ = c;
#else
    // ReSharper disable once CppUseRangeAlgorithm
    std::for_each(arr.begin(), arr.end(), [c = CHAR_MIN](char& c_ref) mutable {
        c_ref = c++;
    });
#endif
    return arr;
}();

template <typename T, char First, char Last, typename Out, typename In>
constexpr auto make_char_table(Out out, In in)
{
    array<T, default_char_table.size()> table{};

    constexpr auto abs_first = -CHAR_MIN + First;
    constexpr auto abs_last  = -CHAR_MIN + Last + 1;

    auto dst = table.begin();

    if constexpr (std::invocable<Out, char>)
        dst = std::transform(default_char_table.begin(), default_char_table.begin() + abs_first, dst, std::ref(out));
    else
        dst = std::fill_n(table.begin(), abs_first, out);
    if constexpr (std::invocable<In, char>)
        dst = std::transform(default_char_table.begin() + abs_first, default_char_table.begin() + abs_last, dst, std::ref(in));
    else
        dst = std::fill_n(dst, abs_last - abs_first, in);
    if constexpr (std::invocable<Out, char>)
        std::transform(default_char_table.begin() + abs_last, default_char_table.end(), dst, std::ref(out));
    else
        std::fill(dst, table.end(), out);

    return table;
}

template <char First, char Last, typename Out, typename In>
constexpr auto make_char_table(Out out, In in)
{
    using array_value = decltype([] {
        if constexpr (std::invocable<Out>)
            return std::invoke_result<Out, char>();
        else
            return std::type_identity<Out>();
    }());

    return make_char_table<typename array_value::type, First, Last>(out, in);
}

template <char First = CHAR_MIN, char Last = CHAR_MAX, typename Fn>
constexpr auto make_char_table(Fn fn)
{
    auto fn_ref = std::ref(fn);
    return make_char_table<std::invoke_result_t<Fn, char>, First, Last>(fn_ref, fn_ref);
}

template <typename T>
constexpr T char_table_get(array<T, default_char_table.size()> const& table, char const c)
{
    return table[-CHAR_MIN + c];
}

template <typename T>
constexpr T& char_table_get(array<T, default_char_table.size()>& table, char const c)
{
    return table[-CHAR_MIN + c];
}

inline constexpr auto islower_table = make_char_table<'a', 'z'>(false, true);
inline constexpr auto isupper_table = make_char_table<'A', 'Z'>(false, true);
inline constexpr auto isdigit_table = make_char_table<'0', '9'>(false, true);

inline constexpr auto isxdigit_table = make_char_table([](char const c) -> bool {
    return is_betweet_two(c, '0', '9') || //
           is_betweet_two(c, 'a', 'f') || //
           is_betweet_two(c, 'A', 'F');
});

inline constexpr auto tolower_table = make_char_table([](char const c) -> char {
    return is_betweet_two(c, 'A', 'F') //
               ? c + ('A' - 'a')
               : c;
});
inline constexpr auto toupper_table = make_char_table([](char const c) -> char {
    return is_betweet_two(c, 'a', 'f') //
               ? c - ('a' - 'A')
               : c;
});

inline constexpr struct
{
    constexpr bool operator()(char const c) const
    {
        return char_table_get(islower_table, c);
    }

    bool operator()(wchar_t) const = delete;
} islower;

inline constexpr struct
{
    constexpr bool operator()(char const c) const
    {
        return char_table_get(isupper_table, c);
    }

    bool operator()(wchar_t) const = delete;
} isupper;

inline constexpr struct
{
    constexpr bool operator()(char const c) const
    {
        return char_table_get(isdigit_table, c);
    }

    bool operator()(wchar_t) const = delete;
} isdigit;

inline constexpr struct
{
    constexpr bool operator()(char const c) const
    {
        return char_table_get(isxdigit_table, c);
    }

    bool operator()(wchar_t) const = delete;
} isxdigit;

//---

inline constexpr struct
{
    constexpr char operator()(char const c) const
    {
        return char_table_get(toupper_table, c);
    }

    wchar_t operator()(wchar_t) const = delete;
} toupper;

inline constexpr struct
{
    constexpr char operator()(char const c) const
    {
        return char_table_get(tolower_table, c);
    }

    wchar_t operator()(wchar_t) const = delete;
} tolower;
} // namespace FD_TIER(1)