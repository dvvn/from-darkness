module;

#include <fds/core/callback.h>

#include <concepts>
#include <format>
#include <functional>
#include <string_view>

export module fds.logger;
export import fds.convert_to;

template <typename S>
using get_char_t = std::remove_cvref_t<decltype(std::declval<S>()[0])>;

template <typename T>
concept can_be_string = requires(const T& obj)
{
    std::basic_string_view(obj);
};

template <typename CharT, typename T>
decltype(auto) _Prepare_fmt_arg(const T& arg)
{
    constexpr auto cvt = fds::convert_to<CharT>;
    if constexpr (can_be_string<T>)
    {
        return cvt(std::basic_string_view(arg));
    }
    else if constexpr (std::invocable<T>)
    {
        auto tmp    = std::invoke(arg);
        using tmp_t = decltype(tmp);
        if constexpr (can_be_string<tmp_t> && !std::same_as<CharT, get_char_t<tmp_t>>)
            return cvt(tmp);
        else
            return tmp;
    }
    else
    {
        return arg;
    }
}

template <typename CharT, typename... Args>
auto _Make_fmt_args(const Args&... args)
{
    if constexpr (std::same_as<CharT, char>)
        return std::make_format_args(args...);
    else if constexpr (std::same_as<CharT, wchar_t>)
        return std::make_wformat_args(args...);
}

FDS_CALLBACK(logger_narrow, std::string_view);
FDS_CALLBACK(logger_wide, std::wstring_view);

template <typename L>
bool _Logger_empty(const L logger)
{
    return !logger.initialized() || logger->empty();
}

template <typename Fn, typename T>
Fn _Wrap_callback(T&& obj)
{
    using obj_t = decltype(obj);
    if constexpr (std::copyable<obj_t> && std::constructible_from<Fn, obj_t>)
        return std::forward<T>(obj);
    else if constexpr (std::is_lvalue_reference_v<obj_t>)
        return [&](const auto msg) {
            return std::invoke(obj, msg);
        };
}

// std::invocable from concepts ignore self overloads!
template <class _FTy, class... _ArgTys>
concept invocable = requires(_FTy&& _Fn, _ArgTys&&... _Args)
{
    std::invoke(static_cast<_FTy&&>(_Fn), static_cast<_ArgTys&&>(_Args)...);
};

class logger_wrapped
{
    void _Log(const std::string_view msg) const
    {
        std::invoke(logger_narrow, msg);
    }

    void _Log_inversed(const std::string_view msg) const
    {
        const std::wstring wide_str(msg.begin(), msg.end());
        _Log(wide_str);
    }

    void _Log(const std::wstring_view msg) const
    {
        std::invoke(logger_wide, msg);
    }

    bool _Log_inversed(const std::wstring_view msg) const
    {
        std::string str;
        str.reserve(msg.size());
        for (const auto wchr : msg)
        {
            const auto chr = static_cast<char>(wchr);
            if (static_cast<wchar_t>(chr) != wchr)
                return false;
            str += chr;
        }
        _Log(str);
        return true;
    }

  public:
    template <typename Fn>
    void append(Fn&& callback) const
    {
        constexpr bool can_narrow = invocable<Fn, std::string_view>;
        constexpr bool can_wide   = invocable<Fn, std::wstring_view>;

        using narrow_fn = callbacks::logger_narrow::callback_type;
        using wide_fn   = callbacks::logger_wide::callback_type;

        if constexpr (can_narrow && can_wide)
        {
            logger_narrow->append(_Wrap_callback<narrow_fn>(callback));
            logger_wide->append(_Wrap_callback<wide_fn>(callback));
        }
        else if constexpr (can_narrow)
            logger_narrow->append(_Wrap_callback<narrow_fn>(std::forward<Fn>(callback)));
        else if constexpr (can_wide)
            logger_wide->append(_Wrap_callback<wide_fn>(std::forward<Fn>(callback)));
        else
            static_assert(std::_Always_false<Fn>);
    }

    bool active() const
    {
        return !_Logger_empty(logger_narrow) || !_Logger_empty(logger_wide);
    }

    bool operator()(const std::string_view msg) const
    {
        if (!_Logger_empty(logger_narrow))
        {
            _Log(msg);
            return true;
        }
        if (!_Logger_empty(logger_wide))
        {
            _Log_inversed(msg);
            return true;
        }
        return false;
    }

    bool operator()(const std::wstring_view msg) const
    {
        if (!_Logger_empty(logger_wide))
        {
            _Log(msg);
            return true;
        }
        return !_Logger_empty(logger_narrow) && _Log_inversed(msg);
    }

    template <std::invocable T>
    bool operator()(T&& fn) const
    {
        if (_Logger_empty(logger_narrow) && _Logger_empty(logger_wide))
            return false;
        const auto msg = std::invoke(fn);
        return std::invoke(*this, msg);
    }

    template <typename Arg1, typename... Args>
    bool operator()(Arg1&& fmt, Args&&... args) const requires(sizeof...(Args) > 0)
    {
        if (_Logger_empty(logger_narrow) && _Logger_empty(logger_wide))
            return false;

        if constexpr (std::invocable<Arg1>)
        {
            const auto fmt_str = std::invoke((fmt));
            using char_t       = get_char_t<decltype(fmt_str)>;
            const auto msg     = std::vformat(fmt_str, _Make_fmt_args<char_t>(_Prepare_fmt_arg<char_t>(args)...));
            return std::invoke(*this, msg);
        }
        else
        {
            using char_t   = get_char_t<decltype(fmt)>;
            const auto msg = std::vformat(fmt, _Make_fmt_args<char_t>(_Prepare_fmt_arg<char_t>(args)...));
            return std::invoke(*this, msg);
        }
    }
};

export namespace fds
{
    constexpr logger_wrapped logger;
}
