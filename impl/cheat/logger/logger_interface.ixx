module;

#include <nstd/format.h>

#include <functional>
#include <string_view>
//#include <sstream>

export module cheat.logger;
export import nstd.text.convert.unicode;

template <typename S>
using get_char_t = std::remove_cvref_t<decltype(std::declval<S>()[0])>;

template <typename T>
concept can_be_string = requires(T obj)
{
    std::basic_string_view(obj);
};

template <typename CharT, typename T>
decltype(auto) _Prepare_fmt_arg(T&& arg)
{
    if constexpr (can_be_string<T>)
    {
        const std::basic_string_view str(arg);
        return nstd::text::convert_to<CharT>(str);
    }
    else if constexpr (std::invocable<T>)
    {
        return _Prepare_fmt_arg<CharT>(std::invoke(arg));
    }
    else
    {
        return (std::forward<T>(arg));
    }
}

template <typename CharT, typename... Args>
auto _Make_fmt_args(Args&&... args)
{
    if constexpr (std::same_as<CharT, char>)
        return nstd::make_format_args(_Prepare_fmt_arg<char>(args)...);
    else if constexpr (std::same_as<CharT, wchar_t>)
        return nstd::make_wformat_args(_Prepare_fmt_arg<wchar_t>(args)...);
}

template <typename CharT, typename Tr, typename... Args>
auto _Vformat(const std::basic_string_view<CharT, Tr> fmt, Args&&... args)
{
    return nstd::vformat(fmt, _Make_fmt_args<CharT>(std::forward<Args>(args)...));
}

class logger
{
  protected:
    virtual void log_impl(const std::string_view str) = 0;
    virtual void log_impl(const std::wstring_view str) = 0;

  public:
    virtual ~logger() = default;

    virtual bool active() const = 0;
    virtual void enable() = 0;
    virtual void disable() = 0;

    void log(const std::string_view str);
    void log(const std::wstring_view str);

    template <std::invocable T>
    void log(T&& fn)
    {
        if (!active())
            return;
        log_impl(std::invoke(std::forward<T>(fn)));
    }

    template <typename Arg1, typename... Args>
    void log(const Arg1& fmt, Args&&... args) requires(sizeof...(Args) > 0)
    {
        if (!active())
            return;
        log_impl(_Vformat(std::basic_string_view(fmt), std::forward<Args>(args)...));
    }
};

export namespace cheat
{
    using ::logger;
}
