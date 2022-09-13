module;

#include <concepts>

export module fd.logger;
export import fd.callback.invoker;
export import fd.format;
import fd.string.utf;

// clang-format off
constexpr void _Char_acceptor(const char){}
constexpr void _Char_acceptor(const wchar_t){}
constexpr void _Char_acceptor(const char8_t){}
constexpr void _Char_acceptor(const char16_t){}
constexpr void _Char_acceptor(const char32_t){}

// clang-format on

template <typename T>
concept can_be_string = requires(const T& obj) { _Char_acceptor(obj[0]); };

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

using namespace fd;

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
        return utf_string<CharT>(obj);
}

struct logs_handler_t : basic_callback_invoker<const string_view>, basic_callback_invoker<const wstring_view>
{
};

export namespace fd
{
    using ::logs_handler_t;

    void invoke(logs_handler_t* l, const string_view msg)
    {
        if (!l)
            return;
        invoke(*l, msg);
    }

    void invoke(logs_handler_t* l, const wstring_view msg)
    {
        if (!l)
            return;
        invoke(*l, msg);
    }

    void invoke(logs_handler_t* l, invocable auto fn)
    {
        if (!l)
            return;
        invoke(*l, invoke(fn));
    }

    template <typename Arg1, typename... Args>
    void invoke(logs_handler_t* l, const Arg1& fmt, const Args&... args) requires(sizeof...(Args) > 0)
    {
        if (!l)
            return;
        const auto fmt_fixed = _Correct_obj(fmt);
        using char_t         = get_char_t<decltype(fmt_fixed)>;
        const auto buff      = format(fmt_fixed, _Correct_obj<char_t>(args)...);
        return invoke(*l, buff);
    }

    logs_handler_t* logger;
} // namespace fd
