module;

#include <ww898/utf_converters.hpp>

export module fd.to_char;

using ww898::utf::detail::utf_selector;

template <>
struct utf_selector<char8_t> final
{
    using type = utf8;
};

template <typename To, typename It, class Alloc = std::allocator<To>>
auto utf_conv(It&& bg, It&& ed, const Alloc = {})
{
    using from = std::remove_cvref_t<decltype(*bg)>;
    static_assert(!std::is_same_v<from, To>);

    using alloc = typename std::allocator_traits<Alloc>::template rebind_alloc<To>;
    std::basic_string<To, std::char_traits<To>, alloc> buff;

    using namespace ww898::utf;
    using sfrom = utf_selector_t<from>;
    using sto   = utf_selector_t<To>;
    conv<sfrom, sto>(std::forward<It>(bg), std::forward<It>(ed), std::back_inserter(buff));

    return buff;
}

template <typename To>
struct to_char_impl
{
    template <typename... S>
    auto operator()(const std::basic_string_view<S...> from) const
    {
        return utf_conv<To>(from.begin(), from.end());
    }

    template <typename... S>
    auto operator()(const std::basic_string<S...>& from) const
    {
        return utf_conv<To>(from.begin(), from.end(), from.get_allocator());
    }

    template <typename C, size_t S>
    auto operator()(const C (&from)[S]) const
    {
        return utf_conv<To, const C*>(from, from + S - 1);
    }

    template <typename C>
    auto operator()(const C from) const requires(std::is_pointer_v<C>) // fix to allow previous overload
    {
        const auto to = from + std::char_traits<std::remove_pointer_t<C>>::length(from);
        return utf_conv<To>(from, to);
    }
};

template <typename To>
constexpr std::nullptr_t to_char;

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
