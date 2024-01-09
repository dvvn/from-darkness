#pragma once

#include "functional/invoke_on.h"
#include "string/view.h"
#include "noncopyable.h"

#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fmt/xchar.h>

namespace fd::detail
{
template <typename CharT, size_t Length, class Config>
class basic_static_string_full;
}

FMT_BEGIN_NAMESPACE

template <typename Fn, typename C>
requires(std::is_bind_expression_v<Fn> && is_formattable<std::invoke_result_t<Fn>, C>::value)
struct formatter<Fn, C> : formatter<std::decay_t<std::invoke_result_t<Fn>>, C>
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

template <class T>
requires requires(T str) {
    []<typename CharT, size_t Length, class Config>(fd::detail::basic_static_string_full<CharT, Length, Config> const&) {
    }(str);
}
struct detail::
#if FMT_VERSION < 100202
    is_string_like
#else
    is_std_string_like
#endif
    <T> : std::true_type
{
};

FMT_END_NAMESPACE

namespace fd
{
template <class Out>
class basic_logger
{
    /*template <class T, class Out1>
    friend T& get(basic_logger<Out1>& logger);

    template <class T, class Out1>
    friend T const& get(basic_logger<Out1> const& logger);*/

    Out out_;

    template <class Fmt, class FmtArgs>
    void write(Fmt fmt_str, FmtArgs fmt_args)
    {
        auto const time = std::chrono::system_clock::now().time_since_epoch();

        decltype(auto) buff = out_.template get_buffer<typename Fmt::value_type>();
        std::back_insert_iterator it{buff};

        fmt::format_to(it, "[{:%H:%M:%S}]", time);
        buff.push_back(' ');
        if constexpr (std::is_null_pointer_v<FmtArgs>)
            std::copy_n(fmt_str.data(), fmt_str.size(), it);
        else
            fmt::vformat_to(it, fmt_str, fmt_args);
        buff.push_back('\n');

        out_.write(buff.data(), buff.size());
    }

  public:
    template <typename... Args>
    void operator()(fmt::format_string<Args...> fmt, Args&&... args) requires(sizeof...(Args) != 0)
    {
        write(fmt.get(), fmt::format_args{fmt::make_format_args(args...)});
    }

    template <typename... Args>
    void operator()(fmt::wformat_string<Args...> fmt, Args&&... args) requires(sizeof...(Args) != 0)
    {
        write(fmt.get(), fmt::wformat_args{fmt::make_wformat_args(args...)});
    }

    void operator()(string_view const str)
    {
        write(str, nullptr);
    }

    void operator()(wstring_view const str)
    {
        write(str, nullptr);
    }

    // for optimization use 'make_constant_string<native_char_type>(XXX)'
#if 1
  private:
    class notification_invoker : public noncopyable
    {
        basic_logger* self_;

      public:
        notification_invoker(basic_logger* self)
            : self_{self}
        {
        }

        void operator()(integral_constant<invoke_on_state, invoke_on_state::construct>) const
        {
            std::invoke(*self_, "Created");
        }

        void operator()(integral_constant<invoke_on_state, invoke_on_state::destruct>) const
        {
            std::invoke(*self_, "Destroyed");
        }
    };

  public:
    [[nodiscard]]
    auto make_notification() -> invoke_on<invoke_on_state::construct | invoke_on_state::destruct, notification_invoker>
    {
        return {this};
    }
#else
    [[nodiscard]]
    auto make_notification()
    {
        return make_invoke_on<invoke_on_state::construct | invoke_on_state::destruct>([this]<invoke_on_state State>(integral_constant<invoke_on_state, State>) {
            if constexpr (State == invoke_on_state::construct)
                write("Created", nullptr);
            else if constexpr (State == invoke_on_state::destruct)
                write("Destroyed", nullptr);
        });
    }
#endif
};

#if 0
template <class Out>
class basic_debug_logger : public basic_logger<Out>
{
  public:
    using basic_logger<Out>::native_char_type;
    using basic_logger<Out>::operator();

    ~basic_debug_logger()
    {
        operator()(make_constant_string<native_char_type>("Destroyed"));
    }

    basic_debug_logger()
    {
        operator()(make_constant_string<native_char_type>("Created"));
    }
};
#endif

struct empty_logger
{
#ifdef _DEBUG
    template <typename... Args>
    void operator()(fmt::format_string<Args...> fmt, Args&&... args) requires(sizeof...(Args) != 0)
    {
    }

    template <typename... Args>
    void operator()(fmt::wformat_string<Args...> fmt, Args&&... args) requires(sizeof...(Args) != 0)
    {
    }
#endif

    template <typename... Args>
    void operator()(Args&&... args)
    {
    }

    static auto notification()
    {
        return std::ignore;
    }
};

/*template <class T, class Out>
T& get(basic_logger<Out>& logger)
{
    return logger.out_;
}

template <class T, class Out>
T const& get(basic_logger<Out> const& logger)
{
    return logger.out_;
}*/
} // namespace fd