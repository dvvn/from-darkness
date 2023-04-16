// ReSharper disable CppRedundantInlineSpecifier
#pragma once

#include <fd/formatting/narrow.h>
#include <fd/formatting/wide.h>

FMT_BEGIN_NAMESPACE

template <typename ContextC, typename StrC, bool Same = std::same_as<ContextC, StrC>>
class chars_mixer;

template <typename C>
class chars_mixer<C, C, true>
{
    basic_string_view<C> cached_;

  public:
    template <typename T>
    FMT_CONSTEXPR chars_mixer(T &&str)
        : cached_(std::forward<T>(str))
    {
    }

    FMT_CONSTEXPR auto native() const
    {
        return cached_;
    }
};

template <typename ContextC, typename StrC>
class chars_mixer<ContextC, StrC, false>
{
    using context_str     = basic_string_view<ContextC>;
    using current_str     = basic_string_view<StrC>;
    using current_str_std = std::basic_string_view<StrC>;

    current_str cached_;

  public:
    static_assert(sizeof(ContextC) >= sizeof(StrC));

    FMT_CONSTEXPR chars_mixer() = default;

    template <typename T>
    FMT_CONSTEXPR chars_mixer(T &&str)
        : cached_(std::forward<T>(str))
    {
    }

    FMT_CONSTEXPR operator current_str() const
    {
        return cached_;
    }

    class buffer
    {
        basic_memory_buffer<ContextC> buffer_;

      public:
        FMT_CONSTEXPR20 buffer(current_str const str)
        {
            buffer_.append(str);
        }

        FMT_CONSTEXPR context_str get() const
        {
            return { buffer_.data(), buffer_.size() };
        }

        FMT_CONSTEXPR operator context_str() const
        {
            return { buffer_.data(), buffer_.size() };
        }
    };

    FMT_CONSTEXPR buffer native() const
    {
        return cached_;
    }
};

template <typename StrC>
class chars_mixer<void, StrC, false>
{
    basic_string_view<StrC> cached_;

  public:
    FMT_CONSTEXPR chars_mixer() = default;

    template <typename T>
    FMT_CONSTEXPR chars_mixer(T &&str)
        : cached_(std::forward<T>(str))
    {
    }

    template <typename ContextC>
    FMT_CONSTEXPR auto native() const
    {
        return chars_mixer<ContextC, StrC>(cached_).native();
    }
};

template <typename C>
struct chars_mixer_formatter : formatter<basic_string_view<C>, C>
{
    template <typename S>
    auto format(chars_mixer<C, S> const &mixer, auto &ctx) const -> decltype(ctx.out())
    {
        using strv = basic_string_view<C>;

        auto buff = mixer.native();
        strv str  = buff;
        return formatter<strv, C>::format(str, ctx);
    }
};

template <typename C>
struct formatter<chars_mixer<C, C, true>, C> : chars_mixer_formatter<C>
{
    using chars_mixer_formatter<C>::format<C>;
};

template <typename ContextC, typename StrC>
struct formatter<chars_mixer<ContextC, StrC, false>, ContextC> : chars_mixer_formatter<ContextC>
{
    using chars_mixer_formatter<ContextC>::format<ContextC>;
};

template <typename ContextC, typename StrC>
struct formatter<chars_mixer<void, StrC, false>, ContextC> : chars_mixer_formatter<ContextC>
{
    using chars_mixer_formatter<ContextC>::format<ContextC>;
};

FMT_BEGIN_DETAIL_NAMESPACE

template <typename ContextC, typename T, bool IsString = is_string<T>::value>
struct chars_mix_requrested;

template <typename ContextC, typename T>
struct chars_mix_requrested<ContextC, T, false> : std::false_type
{
};

template <typename ContextC, typename T>
struct chars_mix_requrested<ContextC, T, true>
{
    using char_type             = char_t<T>;
    static constexpr bool value = sizeof(ContextC) >= sizeof(char_type) && !std::is_same_v<ContextC, char_type>;
};

template <typename T>
struct chars_mix_requrested<void, T, true> : std::true_type
{
};

FMT_END_DETAIL_NAMESPACE

template <typename ContextC, typename T>
concept chars_mix_requrested = detail::chars_mix_requrested<ContextC, T>::value;

template <class ContextC = void, typename T>
FMT_CONSTEXPR decltype(auto) mix_chars(T &obj)
{
    if constexpr (chars_mix_requrested<ContextC, T>)
        return chars_mixer<ContextC, char_t<T>>(obj);
    else
        return static_cast<T &>(obj);
}
template <class ContextC, typename T>
using mix_chars_t = std::decay_t<decltype(mix_chars<ContextC>(std::declval<std::add_lvalue_reference_t<T>>()))>;

FMT_END_NAMESPACE

namespace fd
{
template <typename... Args>
requires(fmt::chars_mix_requrested<char, Args> || ...)
auto to_string(fmt::format_string<fmt::mix_chars_t<char, Args>...> fmt, Args &&...args)
{
    return fmt::vformat(fmt, fmt::make_format_args(fmt::mix_chars<char>(args)...));
}

template <typename... Args>
requires(fmt::chars_mix_requrested<wchar_t, Args> || ...)
auto to_string(fmt::wformat_string<fmt::mix_chars_t<wchar_t, Args>...> fmt, Args &&...args)
{
    return fmt::vformat(fmt, fmt::make_wformat_args(fmt::mix_chars<wchar_t>(args)...));
}
} // namespace fd