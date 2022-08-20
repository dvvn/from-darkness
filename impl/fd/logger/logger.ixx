module;

#include <fd/callback.h>

#include <concepts>

export module fd.logger;
export import fd.utf_convert;
export import fd.format;
export import fd.functional.invoke;

using namespace fd;

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

template <typename S>
using get_char_t = std::remove_cvref_t<decltype(std::declval<S>()[0])>;

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
    if constexpr (invocable<T>)
        return _Correct_obj<CharT>(invoke(obj));
    else if constexpr (std::is_void_v<CharT> || !can_be_string<T>)
        return _Forward_or_move(std::forward<T>(obj));
    else if constexpr (std::is_same_v<get_char_t<T>, CharT>)
        return _Forward_or_move(std::forward<T>(obj));
    else
        return utf_convert<CharT>(obj);
}

FD_CALLBACK(logger_narrow, void, string_view);
FD_CALLBACK(logger_wide, void, wstring_view);

template <typename L>
bool _Logger_empty(const L logger)
{
    return !logger.initialized() || logger->empty();
}

class logger_wrapped
{
    void _Log(const string_view msg) const
    {
        invoke(logger_narrow, msg);
    }

    void _Log_inversed(const string_view msg) const
    {
        const wstring wide_str(msg.begin(), msg.end());
        _Log(wide_str);
    }

    void _Log(const wstring_view msg) const
    {
        invoke(logger_wide, msg);
    }

    bool _Log_inversed(const wstring_view msg) const
    {
        string str;
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
        constexpr bool can_narrow = invocable<Fn, string_view>;
        constexpr bool can_wide   = invocable<Fn, wstring_view>;

        if constexpr (can_narrow && can_wide)
        {
            logger_narrow->push_back(callback);
            logger_wide->push_back(callback);
        }
        else if constexpr (can_narrow)
            logger_narrow->push_back(std::forward<Fn>(callback));
        else if constexpr (can_wide)
            logger_wide->push_back(std::forward<Fn>(callback));
        else
            static_assert(std::_Always_false<Fn>);
    }

    bool active() const
    {
        return !_Logger_empty(logger_narrow) || !_Logger_empty(logger_wide);
    }

    bool operator()(const string_view msg) const
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

    bool operator()(const wstring_view msg) const
    {
        if (!_Logger_empty(logger_wide))
        {
            _Log(msg);
            return true;
        }
        return !_Logger_empty(logger_narrow) && _Log_inversed(msg);
    }

    template <invocable T>
    bool operator()(T fn) const
    {
        if (_Logger_empty(logger_narrow) && _Logger_empty(logger_wide))
            return false;
        return invoke(*this, invoke(fn));
    }

    template <typename Arg1, typename... Args>
    bool operator()(const Arg1& fmt, const Args&... args) const requires(sizeof...(Args) > 0)
    {
        if (_Logger_empty(logger_narrow) && _Logger_empty(logger_wide))
            return false;

        const auto fmt_fixed = _Correct_obj(fmt);
        using char_t         = get_char_t<decltype(fmt_fixed)>;
        const auto buff      = format(fmt_fixed, _Correct_obj<char_t>(args)...);
        return invoke(*this, buff);
    }
};

export namespace fd
{
    FD_OBJECT(logger, logger_wrapped);
} // namespace fd
