module;

#if __has_include(<fmt/format.h>) && defined(_MSC_VER) && _MSC_VER <= 1932
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fmt/xchar.h>
#else
#include <format>
#endif

export module fd.format;
export import fd.string;

#ifdef FMT_VERSION
#define _FMT fmt
#else
#define _FMT std

// compiler stuck without it
export namespace std::inline literals::inline string_view_literals
{
    using ::std::string_view_literals::operator"" sv;
}
#endif

/* namespace _FMT
{
    template <typename C>
    struct formatter<fd::basic_string_view<C>, C> : formatter<basic_string_view<C>, C>
    {
        template <class FormatContext>
        auto format(const fd::basic_string_view<C> str, FormatContext& fc) const
        {
            const basic_string_view<C> tmp(str.data(), str.size());
            return formatter<basic_string_view<C>, C>::format(tmp, fc);
        }
    };

    template <typename C>
    struct formatter<fd::basic_string<C>, C> : formatter<fd::basic_string_view<C>, C>
    {
    };
} // namespace _FMT */

class format_to_impl
{
    template <typename C, typename... Args>
    static auto make_format_args(const Args&... args)
    {
        if constexpr (std::same_as<C, char>)
            return _FMT::make_format_args(args...);
        else if constexpr (std::same_as<C, wchar_t>)
            return _FMT::make_wformat_args(args...);
    }

    template <typename Out, class S, typename... Args>
    static auto impl(Out out, const S fmt, const Args&... args)
    {
        static_assert(sizeof...(Args) > 0);
        _FMT::vformat_to(out, fmt, make_format_args<typename S::value_type>(args...));
    }

  public:
    template <typename Out, typename... Args>
    auto operator()(Out out, const fd::string_view fmt, const Args&... args) const
    {
        return impl(out, fmt, args...);
    }

    template <typename Out, typename... Args>
    auto operator()(Out out, const fd::wstring_view fmt, const Args&... args) const
    {
        return impl(out, fmt, args...);
    }
};

class format_impl
{
    [[no_unique_address]] format_to_impl format_to_;

    template <typename S, typename... Args>
    auto impl(const S fmt, const Args&... args) const
    {
        fd::basic_string<typename S::value_type> buff;
        format_to_(std::back_inserter(buff), fmt, args...);
        return buff;
    }

  public:
    template <typename... Args>
    auto operator()(const fd::string_view fmt, const Args&... args) const
    {
        return impl(fmt, args...);
    }

    template <typename... Args>
    auto operator()(const fd::wstring_view fmt, const Args&... args) const
    {
        return impl(fmt, args...);
    }
};

export namespace fd
{
    constexpr format_to_impl format_to;
    constexpr format_impl format;
} // namespace fd
