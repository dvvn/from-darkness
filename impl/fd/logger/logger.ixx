module;

#include <fd/callback.h>

#include <concepts>
#include <functional>

export module fd.logger;
export import fd.to_char;
export import fd.format;

template <typename T>
concept have_array_access = requires(const T& obj)
{
    obj[0];
};

struct dummy
{
};

template <typename S>
auto _Get_char_t()
{
    if constexpr (have_array_access<S>)
        return std::remove_cvref_t<decltype(std::declval<S>()[0])>();
    else
        return dummy();
}

template <typename S>
using get_char_t = decltype(_Get_char_t<S>());

// template <typename T>
// concept correct_char_traits = !std::same_as<decltype(std::declval<T>())::int_type, unsigned long>;

// clang-format off
constexpr void _Char_acceptor(const char){}
constexpr void _Char_acceptor(const wchar_t){}
constexpr void _Char_acceptor(const char8_t){}
constexpr void _Char_acceptor(const char16_t){}
constexpr void _Char_acceptor(const char32_t){}

// clang-format on

template <typename T>
concept can_be_string = requires(const T& obj)
{
    _Char_acceptor(obj[0]);
};

template <typename T>
decltype(auto) _Forward_or_move(T&& obj)
{
    if constexpr (std::is_rvalue_reference_v<decltype(obj)>)
        return std::remove_cvref_t<T>(std::forward<T>(obj));
    else
        return std::forward<T>(obj);
}

template <typename CharT = void, typename T>
decltype(auto) _Correct_obj(T&& obj)
{
    if constexpr (std::invocable<T>)
        return _Correct_obj<CharT>(std::invoke(obj));
    else if constexpr (std::is_same_v<get_char_t<T>, CharT> || std::is_void_v<CharT> || !can_be_string<T>)
        return _Forward_or_move(std::forward<T>(obj));
    else
        return fd::to_char<CharT>(obj);
}

FD_CALLBACK(logger_narrow, fd::string_view);
FD_CALLBACK(logger_wide, fd::wstring_view);

namespace std
{
    void invoke(decltype(logger_narrow) l, const fd::string_view str)
    {
        invoke(*l, str);
    }

    void invoke(decltype(logger_wide) l, const fd::wstring_view wstr)
    {
        invoke(*l, wstr);
    }
} // namespace std

template <typename L>
bool _Logger_empty(const L logger)
{
    return !logger.initialized() || logger->empty();
}

// std::invocable from concepts ignore self overloads!
template <class _FTy, class... _ArgTys>
concept invocable = requires(_FTy&& _Fn, _ArgTys&&... _Args)
{
    std::invoke(static_cast<_FTy&&>(_Fn), static_cast<_ArgTys&&>(_Args)...);
};

class logger_wrapped
{
    void _Log(const fd::string_view msg) const
    {
        std::invoke(logger_narrow, msg);
    }

    void _Log_inversed(const fd::string_view msg) const
    {
        const fd::wstring wide_str(msg.begin(), msg.end());
        _Log(wide_str);
    }

    void _Log(const fd::wstring_view msg) const
    {
        std::invoke(logger_wide, msg);
    }

    bool _Log_inversed(const fd::wstring_view msg) const
    {
        fd::string str;
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
        constexpr bool can_narrow = invocable<Fn, fd::string_view>;
        constexpr bool can_wide   = invocable<Fn, fd::wstring_view>;

        if constexpr (can_narrow && can_wide)
        {
            logger_narrow->append(callback);
            logger_wide->append(callback);
        }
        else if constexpr (can_narrow)
            logger_narrow->append(std::forward<Fn>(callback));
        else if constexpr (can_wide)
            logger_wide->append(std::forward<Fn>(callback));
        else
            static_assert(std::_Always_false<Fn>);
    }

    bool active() const
    {
        return !_Logger_empty(logger_narrow) || !_Logger_empty(logger_wide);
    }

    bool operator()(const fd::string_view msg) const
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

    bool operator()(const fd::wstring_view msg) const
    {
        if (!_Logger_empty(logger_wide))
        {
            _Log(msg);
            return true;
        }
        return !_Logger_empty(logger_narrow) && _Log_inversed(msg);
    }

    template <std::invocable T>
    bool operator()(T fn) const
    {
        if (_Logger_empty(logger_narrow) && _Logger_empty(logger_wide))
            return false;
        return std::invoke(*this, std::invoke(fn));
    }

    template <typename Arg1, typename... Args>
    bool operator()(const Arg1& fmt, const Args&... args) const requires(sizeof...(Args) > 0)
    {
        if (_Logger_empty(logger_narrow) && _Logger_empty(logger_wide))
            return false;

        const auto fmt_fixed = _Correct_obj(fmt);
        using char_t         = get_char_t<decltype(fmt_fixed)>;
        const auto buff      = fd::format(fmt_fixed, _Correct_obj<char_t>(args)...);
        return std::invoke(*this, buff);
    }
};

export namespace fd
{
    constexpr logger_wrapped logger;
}

/* export namespace std
{
    template <typename... Args>
    void invoke(const logger_wrapped l, const Args&... args)
    {
        l(args...);
    }
} // namespace std */
