#pragma once

// #include <fd/string.h>

#include <type_traits>

#if __has_include(<fmt/format.h>)

#ifdef __cpp_lib_format
// ReSharper disable CppInconsistentNaming
namespace std
{
template <class _CharT, class... _Args>
struct _Basic_format_string;

template <class... _Args>
struct _Basic_format_string<char, _Args...>
{
    _Basic_format_string() = delete;
};

template <class... _Args>
struct _Basic_format_string<wchar_t, _Args...>
{
    _Basic_format_string() = delete;
};

} // namespace std

// ReSharper restore CppInconsistentNaming
#endif

#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fmt/xchar.h>
#define _FMT fmt
#else
#include <format>
#define _FMT std
#endif

/* namespace _FMT
{
    template <typename C>
    struct formatter<fd::basic_string_view<C>, C> : formatter<basic_string_view<C>, C>
    {
        template <class FormatContext>
        auto format(const fd::basic_string_view<C> str, FormatContext& fc) const
        {
            const basic_string_view<C> tmp(str.data(), str.size());
            return formatter<basic_string_view<C>, C>::format(tmp, fc);
        }
    };

    template <typename C>
    struct formatter<fd::basic_string<C>, C> : formatter<fd::basic_string_view<C>, C>
    {
    };
} // namespace _FMT */

namespace fd
{
using _FMT::formatter;
using _FMT::to_string;
using _FMT::to_wstring;

#ifdef FMT_VERSION
using fmt::format_int;
#endif

#if 1
using _FMT::format;
using _FMT::format_to;
#else

template <typename C, typename... Args>
static auto _make_format_args(const Args&... args)
{
    if constexpr (std::same_as<C, char>)
        return _FMT::make_format_args(args...);
    else if constexpr (std::same_as<C, wchar_t>)
        return _FMT::make_wformat_args(args...);
}

template <typename Out, class C, typename... Args>
static auto _format_to(Out out, const basic_string_view<C> fmt, const Args&... args)
{
    static_assert(sizeof...(Args) > 0);
    _FMT::vformat_to(out, fmt, _make_format_args<C>(args...));
}

template <typename Out, typename... Args>
auto format_to(Out out, const string_view fmt, const Args&... args)
{
    return _format_to(out, fmt, args...);
}

template <typename Out, typename... Args>
auto format_to(Out out, const wstring_view fmt, const Args&... args)
{
    return _format_to(out, fmt, args...);
}

template <typename C, typename... Args>
static auto _format(const basic_string_view<C> fmt, const Args&... args)
{
    basic_string<C> buff;
    _format_to(std::back_insert_iterator(buff), fmt, args...);
    return buff;
}

template <typename... Args>
auto format(const string_view fmt, const Args&... args)
{
    return _format(fmt, args...);
}

template <typename... Args>
auto format(const wstring_view fmt, const Args&... args)
{
    return _format(fmt, args...);
}
#endif
} // namespace fd

#undef _FMT