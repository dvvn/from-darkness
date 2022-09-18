module;

#include <concepts>

export module fd.logger;
export import fd.format;
export import fd.functional.invoke;
import fd.string.utf;

export namespace fd
{
    struct _Char_acceptor
    {
        constexpr _Char_acceptor(const char)
        {
        }

        constexpr _Char_acceptor(const wchar_t)
        {
        }

        constexpr _Char_acceptor(const char8_t)
        {
        }

        constexpr _Char_acceptor(const char16_t)
        {
        }

        constexpr _Char_acceptor(const char32_t)
        {
        }
    };

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

    //---

    export struct basic_logs_handler
    {
        virtual ~basic_logs_handler()                         = default;
        virtual void operator()(const string_view msg) const  = 0;
        virtual void operator()(const wstring_view msg) const = 0;
    };

    export void invoke(basic_logs_handler* l, const string_view msg)
    {
        if (!l)
            return;
        invoke(*l, msg);
    }

    export void invoke(basic_logs_handler* l, const wstring_view msg)
    {
        if (!l)
            return;
        invoke(*l, msg);
    }

    export void invoke(basic_logs_handler* l, invocable auto fn)
    {
        if (!l)
            return;
        invoke(*l, invoke(fn));
    }

    export template <typename Arg1, typename... Args>
    void invoke(basic_logs_handler* l, const Arg1& fmt, const Args&... args) requires(sizeof...(Args) > 0)
    {
        if (!l)
            return;
        const auto fmt_fixed = _Correct_obj(fmt);
        using char_t         = get_char_t<decltype(fmt_fixed)>;
        const auto buff      = format(fmt_fixed, _Correct_obj<char_t>(args)...);
        return invoke(*l, buff);
    }

    basic_logs_handler* logger;
} // namespace fd
