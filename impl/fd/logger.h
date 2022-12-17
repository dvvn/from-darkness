#pragma once

#include <fd/format.h>
#include <fd/functional.h>
#include <fd/utf_string.h>

#include <concepts>

namespace fd
{
    template <typename T, typename... Args>
    static constexpr bool _has_type()
    {
        return (std::same_as<T, Args> || ...);
    }

    template <typename T>
    concept is_character = _has_type<
        std::remove_cvref_t<T>,
        char,
#ifdef __cpp_lib_char8_t
        char8_t,
#endif
        wchar_t,
        char16_t,
        char32_t>();

    template <typename T>
    concept can_be_string = requires(const T& obj) {
                                {
                                    obj[0]
                                    } -> is_character;
                            };

    template <typename S>
    using get_char_t = std::remove_cvref_t<decltype(std::declval<S>()[0])>;

    template <typename T>
    static decltype(auto) _forward_or_move(T&& obj)
    {
        if constexpr (std::is_rvalue_reference_v<decltype(obj)>)
            return std::remove_cvref_t<T>(std::forward<T>(obj));
        else
            return std::forward<T>(obj);
    }

    template <typename CharT = void, typename T>
    static decltype(auto) _correct_obj(T&& obj)
    {
        if constexpr (invocable<T>)
            return _correct_obj<CharT>(invoke(obj));
        else if constexpr (std::is_void_v<CharT> || !can_be_string<T>)
            return _forward_or_move(std::forward<T>(obj));
        else if constexpr (std::is_same_v<get_char_t<T>, CharT>)
            return _forward_or_move(std::forward<T>(obj));
        else
            return utf_string<CharT>(obj);
    }

    //---

    struct basic_logs_handler
    {
        virtual ~basic_logs_handler() = default;

        virtual void operator()(string_view msg) const  = 0;
        virtual void operator()(wstring_view msg) const = 0;
    };

    inline void invoke(basic_logs_handler* l, const string_view msg)
    {
        if (!l)
            return;
        invoke(*l, msg);
    }

    inline void invoke(basic_logs_handler* l, const wstring_view msg)
    {
        if (!l)
            return;
        invoke(*l, msg);
    }

    void invoke(basic_logs_handler* l, invocable auto fn)
    {
        if (!l)
            return;
        invoke(*l, invoke(fn));
    }

    template <typename Arg1, typename... Args>
    void invoke(basic_logs_handler* l, const Arg1& fmt, const Args&... args) requires(sizeof...(Args) > 0)
    {
        if (!l)
            return;
        decltype(auto) fmtFixed = _correct_obj(fmt);
        using char_t            = get_char_t<decltype(fmtFixed)>;
        const auto buff         = fd::format(fmtFixed, _correct_obj<char_t>(args)...);
        return invoke(*l, buff);
    }

    extern basic_logs_handler* Logger;
} // namespace fd