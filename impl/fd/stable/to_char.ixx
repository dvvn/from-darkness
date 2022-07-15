module;

#include <ww898/utf_converters.hpp>

export module fd.to_char;
export import fd.string;

using ww898::utf::detail::utf_selector;

template <>
struct utf_selector<char8_t> final
{
    using type = utf8;
};

template <typename To, typename It>
auto utf_conv(It&& bg, It&& ed)
{
    using from = std::remove_cvref_t<decltype(*bg)>;
    static_assert(!std::is_same_v<from, To>);

    fd::basic_string<To> buff;

    using namespace ww898::utf;
    using sfrom = utf_selector_t<from>;
    using sto   = utf_selector_t<To>;
    conv<sfrom, sto>(std::forward<It>(bg), std::forward<It>(ed), std::back_inserter(buff));

    return buff;
}

template <typename To>
struct to_char_impl
{
    template <typename C>
    auto operator()(const fd::basic_string_view<C> from) const
    {
        return utf_conv<To>(from.begin(), from.end());
    }

    template <typename C>
    auto operator()(const fd::basic_string<C>& from) const
    {
        return utf_conv<To>(from.begin(), from.end());
    }

    template <typename C, size_t S>
    auto operator()(const C (&from)[S]) const
    {
        return utf_conv<To, const C*>(from, from + S - 1);
    }

    template <typename C>
    auto operator()(const C from) const requires(std::is_pointer_v<C>) // fix to allow previous overload
    {
        const auto size = fd::basic_string_view(from).size();
        const auto to   = from + size;
        return utf_conv<To>(from, to);
    }
};

template <typename To>
constexpr auto to_char = nullptr;

#define TO_CHAR(_T_) \
    template <>      \
    constexpr to_char_impl<_T_> to_char<_T_>;

TO_CHAR(char);
TO_CHAR(char8_t);
TO_CHAR(wchar_t);
TO_CHAR(char16_t);
TO_CHAR(char32_t);

export namespace fd
{
    using ::to_char;
}
