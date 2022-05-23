module;

#include <nstd/format.h>

#include <functional>
#include <string_view>
//#include <sstream>

export module cheat.logger;
import nstd.text.convert.unicode;

template <typename S>
using get_char_t = std::remove_cvref_t<decltype(std::declval<S>()[0])>;

struct string_accepter
{
    template <typename... Ts>
    string_accepter(const std::basic_string_view<Ts...>)
    {
    }

    template <typename... Ts>
    string_accepter(const std::basic_string<Ts...>)
    {
    }

    template <typename T>
    string_accepter(const T*)
    {
    }

    template <typename T, size_t S>
    string_accepter(const T (&)[S])
    {
    }
};

template <typename T>
concept string_like = std::constructible_from<string_accepter, T>;

template <typename T>
decltype(auto) _To_string_view(T&& obj) noexcept
{
    if constexpr (string_like<T>)
    {
        if constexpr (std::is_rvalue_reference_v<decltype(obj)>)
            return std::basic_string(std::move(obj));
        else
            return std::basic_string_view(obj);
    }
    else if constexpr (std::invocable<T>)
    {
        return _To_string_view(std::invoke(std::forward<T>(obj)));
    }
    else
    {
        return std::forward<T>(obj);
    }
}

template <typename To, typename From>
decltype(auto) _Convert_to(From&& obj) noexcept
{
    if constexpr (!string_like<From>)
    {
        return std::forward<From>(obj);
    }
    else
    {
        using char_t = get_char_t<From>;
        if constexpr (!std::same_as<To, char_t>)
            return nstd::text::convert_to<To>(obj);
        else if constexpr (std::is_rvalue_reference_v<decltype(obj)>)
            return From(std::forward<From>(obj));
        else
            return obj;
    }
}

template <typename CharT, typename Arg>
decltype(auto) _Prepare_fmt_arg(Arg&& arg)
{
    return _Convert_to<CharT>(_To_string_view(std::forward<Arg>(arg)));
}

template <typename CharT, typename... Args>
auto _Make_fmt_args(Args&&... args)
{
    if constexpr (std::same_as<CharT, char>)
        return nstd::make_format_args(_Prepare_fmt_arg<char>(std::forward<Args>(args))...);
    else if constexpr (std::same_as<CharT, wchar_t>)
        return nstd::make_wformat_args(_Prepare_fmt_arg<wchar_t>(std::forward<Args>(args))...);
}

template <typename CharT, typename Tr, typename... Args>
auto _Vformat(const std::basic_string_view<CharT, Tr> fmt, Args&&... args)
{
    return nstd::vformat(fmt, _Make_fmt_args<CharT>(std::forward<Args>(args)...));
}

class logger
{
  protected:
    virtual void log_impl(const std::string_view str) noexcept = 0;
    virtual void log_impl(const std::wstring_view str) noexcept = 0;

  public:
    virtual ~logger() = default;

    virtual bool active() const noexcept = 0;
    virtual void enable() = 0;
    virtual void disable() = 0;

    void log(const std::string_view str) noexcept;
    void log(const std::wstring_view str) noexcept;

    template <std::invocable T>
    void log(T&& fn) noexcept
    {
        if (!active())
            return;
        log_impl(std::invoke(std::forward<T>(fn)));
    }

    template <typename Arg1, typename... Args>

    requires(sizeof...(Args) > 0) void log(const Arg1& fmt, Args&&... args) noexcept
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

module :private;

void logger::log(const std::string_view str) noexcept
{
    if (!active())
        return;
    log_impl(str);
}

void logger::log(const std::wstring_view str) noexcept
{
    if (!active())
        return;
    log_impl(str);
}
