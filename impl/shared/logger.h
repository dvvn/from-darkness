#pragma once

#include "container/vector/small.h"
#include "type_traits/conditional.h"

#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fmt/xchar.h>

template <typename Fn, typename C>
requires(std::is_bind_expression_v<Fn> && fmt::is_formattable<std::invoke_result_t<Fn>, C>::value)
struct fmt::formatter<Fn, C> : formatter<std::decay_t<std::invoke_result_t<Fn>>, C>
{
    template <typename FnFwd>
    auto format(FnFwd&& binder, auto& ctx) const -> decltype(ctx.out())
#ifdef _DEBUG
        requires(std::same_as<std::remove_cvref_t<FnFwd>, Fn>)
#endif
    {
        using ret_t = std::invoke_result_t<Fn>;
        return formatter<std::decay_t<ret_t>, C>::format(std::invoke(std::forward<FnFwd>(binder)), ctx);
    }
};

namespace fd
{
template <class Out>
class basic_logger
{
    template <class T, class Out1>
    friend T& get(basic_logger<Out1>& logger);

    template <class T, class Out1>
    friend T const& get(basic_logger<Out1> const& logger);

    Out out_;

    template <typename C, class FmtArgs>
    void write(fmt::basic_string_view<C> fmt_str, FmtArgs fmt_args)
    {
        auto const time = std::chrono::system_clock::now().time_since_epoch();

        using buff_char_t = conditional_t<std::same_as<C, char> && std::same_as<typename Out::native_char_type, wchar_t>, wchar_t, C>;
        small_vector<buff_char_t, 512> buff;
        auto it = std::back_inserter(buff);

        fmt::format_to(it, "[{:%H:%M:%S}]", time);
        buff.push_back(' ');
        fmt::vformat_to(it, fmt_str, fmt_args);
        buff.push_back('\n');

        out_.write(buff.data(), buff.size());
    }

  public:
    template <typename... Args>
    void operator()(fmt::format_string<Args...> fmt, Args&&... args)
    {
        write(fmt.get(), fmt::format_args{fmt::make_format_args(args...)});
    }

    template <typename... Args>
    void operator()(fmt::wformat_string<Args...> fmt, Args&&... args)
    {
        write(fmt.get(), fmt::wformat_args{fmt::make_wformat_args(args...)});
    }

    /*Out* operator->()
    {
        return &out_;
    }

    Out const* operator->() const
    {
        return &out_;
    }*/
};

struct fake_logger
{
#ifdef _DEBUG
    template <typename... Args>
    void operator()(fmt::format_string<Args...> fmt, Args&&... args)
    {
    }

    template <typename... Args>
    void operator()(fmt::wformat_string<Args...> fmt, Args&&... args)
    {
    }
#else
    template <typename... Args>
    void operator()(Args&&... args)
    {
    }
#endif
};

template <class T, class Out>
T& get(basic_logger<Out>& logger)
{
    return logger.out_;
}

template <class T, class Out>
T const& get(basic_logger<Out> const& logger)
{
    return logger.out_;
}
} // namespace fd