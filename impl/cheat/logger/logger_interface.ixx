module;

#include <nstd/format.h>

#include <functional>
#include <string_view>
//#include <sstream>

export module cheat.logger;
export import nstd.text.convert.unicode;

template <typename S>
using get_char_t = std::remove_cvref_t<decltype(std::declval<S>()[0])>;

struct to_string_view_t
{
    template <typename... Ts>
    auto operator()(const std::basic_string_view<Ts...> str) const
    {
        return str;
    }

    template <typename C, typename Tr, class A>
    auto operator()(const std::basic_string<C, Tr, A>& str) const
    {
        return std::basic_string_view<C, Tr>(str.begin(), str.end());
    }

    template <typename... Ts>
    auto operator()(std::basic_string<Ts...>&& str) const
    {
        return std::move(str);
    }

    template <typename C>
    auto operator()(const C* str) const
    {
        return std::basic_string_view<C>(str);
    }

    // template <typename C, size_t S>
    // auto operator()(const C (&str)[S]) const
    // {
    //     return std::basic_string_view<C>(str);
    // }
};

constexpr to_string_view_t to_string_view;

template <typename T>
concept can_be_string = requires(T obj)
{
    to_string_view(obj);
};

template <typename CharT, typename T>
decltype(auto) _Prepare_fmt_arg(T&& arg)
{
    if constexpr (can_be_string<T>)
        return nstd::text::convert_to<CharT>(to_string_view(std::forward<T>(arg)));
    else if constexpr (std::invocable<T>)
        return _Prepare_fmt_arg<CharT>(std::invoke(arg));
    else
        return std::forward<T>(arg);
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
    void log(Arg1&& fmt, Args&&... args) requires(sizeof...(Args) > 0)
    {
        if (!active())
            return;
        log_impl(_Vformat(to_string_view(fmt), std::forward<Args>(args)...));
    }
};

export namespace cheat
{
    using ::logger;
}
