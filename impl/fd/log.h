#pragma once

#include "core.h"

#include <fmt/format.h>
#include <fmt/xchar.h>

#include <iosfwd>

#ifdef _DEBUG
FMT_BEGIN_NAMESPACE

using u8string_view = basic_string_view<char8_t>;

template <typename... Args>
using u8format_string = basic_format_string<char8_t, type_identity_t<Args>...>;

template <std::convertible_to<u8string_view> T>
struct formatter<T> : formatter<string_view>
{
    auto format(u8string_view str, format_context &ctx) const -> format_context::iterator
    {
        return formatter<string_view>::format(reinterpret_cast<string_view &>(str), ctx);
    }
};

template <std::convertible_to<string_view> T>
struct formatter<T, char8_t> : private formatter<string_view>
{
    using char_pointer = std::add_const_t<char8_t> *;

    auto parse(auto &ctx) -> char_pointer
    {
        auto end = formatter<string_view>::parse(reinterpret_cast<format_context::parse_context_type &>(ctx));
        return reinterpret_cast<char_pointer>(end);
    }

    auto format(string_view str, auto &ctx) const -> decltype(ctx.out())
    {
        formatter<string_view>::format(str, reinterpret_cast<format_context &>(ctx));
        return ctx.out();
    }
};

FMT_END_NAMESPACE
#endif

namespace fd
{
class logging_activator : public noncopyable
{
    int prev_mode_;
    bool prev_sync_;

  public:
    ~logging_activator();
    logging_activator();

    explicit operator bool() const;
};

#ifdef _DEBUG
void log(fmt::string_view fmt, fmt::format_args fmt_args = {}, std::ostream *out = nullptr);
void log(fmt::wstring_view fmt, fmt::wformat_args fmt_args = {}, std::wostream *out = nullptr);

template <typename... Args>
void log(fmt::u8format_string<Args...> fmt, Args &&...args)
{
    auto in = fmt.get();
    log(reinterpret_cast<fmt::string_view &>(in), fmt::format_args(fmt::make_format_args(args...)));
}

template <typename... Args>
void log(fmt::format_string<Args...> fmt, Args &&...args)
{
    log(fmt.get(), fmt::format_args(fmt::make_format_args(args...)));
}

template <typename... Args>
void log(fmt::wformat_string<Args...> fmt, Args &&...args)
{
    log(fmt.get(), fmt::wformat_args(fmt::make_wformat_args(args...)));
}
#else
void log(...)
{
}
#endif

} // namespace fd