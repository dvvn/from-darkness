#pragma once

#if 1
#include "container/span.h"
#else
#include "type_traits/small_type.h"

#define BOOST_STATIC_STRING_STANDALONE
#include <boost/static_string.hpp>
#endif

#include <cassert>

namespace fd
{
template <typename CharT_l, typename CharT_r>
struct string_join_char : conditional<sizeof(CharT_l) >= sizeof(CharT_r), CharT_l, CharT_r>
{
};

template <typename CharT_l, typename CharT_r>
using string_join_char_t = typename string_join_char<CharT_l, CharT_r>::type;

#ifdef BOOST_STATIC_STRING_HPP
using boost::static_strings::basic_static_string;

template <size_t Length, typename T>
constexpr auto size(basic_static_string<Length, T> const&) -> integral_constant<small_type<T, Length>, Length>
{
    return {};
}
#else
namespace detail
{
template <class It, typename Ptr>
using select_iterator = conditional_t<
#ifdef _MSC_VER
    _ITERATOR_DEBUG_LEVEL >= 1
#else
    sizeof(It) != sizeof(Ptr)
#endif
        && std::is_trivially_destructible_v<It>,
    It,
    Ptr>;
} // namespace detail

template <size_t Length, typename CharT>
struct basic_static_cstring; // null terminated static string

template <size_t Length, typename CharT>
class basic_static_string
{
    using buffer_type = CharT[Length];

    template <typename It>
    constexpr void assign_internal(It first, It last)
    {
#ifdef _DEBUG
        auto const old_size = size_;
#endif
        auto const buffer_first = std::begin(buffer_);
        auto const buffer_last  = std::copy(first, last, buffer_first);
        size_                   = static_cast<size_type>(std::distance(buffer_first, buffer_last));
#ifdef _DEBUG
        if (size_ < old_size)
            std::fill(buffer_last, std::end(buffer_), static_cast<CharT>('\0'));
#endif
    }

    constexpr void pre_construct() noexcept
    {
#ifdef _DEBUG
        size_ = 0;
#endif
    }

  public:
    using size_type       = small_type<size_t, Length>;
    using difference_type = /*small_type<ptrdiff_t, Length>*/ ptrdiff_t;

    using value_type      = CharT;
    using pointer         = CharT*;
    using const_pointer   = CharT const*;
    using reference       = CharT&;
    using const_reference = CharT const&;

    using iterator       = detail::select_iterator<typename span<CharT>::iterator, pointer>;
    using const_iterator = detail::select_iterator<typename span<CharT const>::iterator, const_pointer>;

    buffer_type buffer_;
    size_type size_;

    constexpr basic_static_string()
        : buffer_{}
        , size_{0}
    {
    }

    template <typename It>
    constexpr basic_static_string(It first, It last)
    {
        pre_construct();
        assign_internal(first, last);
    }

    template <size_t L, typename C>
    constexpr basic_static_string(C const (&source)[L])
    {
        pre_construct();
        assign_internal(source, std::next(source, L - 1));
    }

    template <std::incrementable It>
    constexpr basic_static_string(It source)
    {
        pre_construct();
        assign_internal(source, std::next(source, Length));
    }

    template <typename It>
    constexpr basic_static_string& assign(It first, It last)
    {
        assign_internal(first, last);
        return *this;
    }

    constexpr pointer data() noexcept
    {
        return std::data(buffer_);
    }

    constexpr const_pointer data() const noexcept
    {
        return std::data(buffer_);
    }

    constexpr size_type size() const noexcept
    {
        return size_;
    }

    constexpr bool empty() const noexcept
    {
        return size_ == 0;
    }

    constexpr size_type length() const noexcept
    {
        return size_;
    }

    static integral_constant<size_type, Length> max_size() noexcept
    {
        return {};
    }

    constexpr iterator begin() noexcept
    {
        if constexpr (std::is_pointer_v<iterator>)
            return data();
        else
            return span{data(), size_}.begin();
    }

    constexpr const_iterator begin() const noexcept
    {
        if constexpr (std::is_pointer_v<const_iterator>)
            return data();
        else
            return span{data(), size_}.begin();
    }

    constexpr iterator end() noexcept
    {
        if constexpr (std::is_pointer_v<iterator>)
            return data() + size_;
        else
            return span{data(), size_}.end();
    }

    constexpr const_iterator end() const noexcept
    {
        if constexpr (std::is_pointer_v<const_iterator>)
            return data() + size_;
        else
            return span{data(), size_}.end();
    }

#ifdef _MSC_VER
    // ReSharper disable CppInconsistentNaming

    constexpr pointer _Unchecked_begin() noexcept
    {
        return data();
    }

    constexpr const_pointer _Unchecked_begin() const noexcept
    {
        return data();
    }

    constexpr pointer _Unchecked_end() noexcept
    {
        return data() + size_;
    }

    constexpr const_pointer _Unchecked_end() const noexcept
    {
        return data() + size_;
    }

    // ReSharper restore CppInconsistentNaming
#endif

    constexpr void push_back(value_type chr)
    {
        assert(size_ < Length);
        buffer_[size_] = chr;
        ++size_;
    }

    template <typename It>
    constexpr basic_static_string& append(It first, It last)
    {
        auto const extra_size = std::distance(first, last);
        assert(size_ + extra_size <= Length);
        std::copy(first, last, buffer_ + size_);
        size_ += static_cast<size_type>(extra_size);
        return *this;
    }

    template <typename It>
    constexpr basic_static_string& append(It first, size_type count)
    {
        assert(size_ + count <= Length);
        std::copy_n(first, count, buffer_ + size_);
        size_ += count;
        return *this;
    }

    template <typename Rng>
    constexpr basic_static_string& append(Rng const& rng)
#ifdef _DEBUG
        requires requires { std::data(rng); }
#endif
    {
        using std::data;
        using std::size;

        auto rng_size = size(rng);
        if constexpr (std::is_bounded_array_v<Rng>)
        {
            assert(rng[rng_size - 1] == '\0');
            --rng_size;
        }

        return append(data(rng), rng_size);
    }
};

#if 0
template <class Result, class... S>
struct string_join_result;

template <size_t Length, typename CharT, class... S>
struct string_join_result<CharT[Length], S...> : string_join_result<basic_static_string<Length - 1, std::remove_const_t<CharT> >, S...>
{
};

template <size_t Length, typename CharT>
struct string_join_result<basic_static_string<Length, CharT> > : type_identity<basic_static_string<Length, CharT> >
{
};

template <size_t Length, typename CharT>
struct string_join_result<basic_static_string<Length, CharT> const> : type_identity<basic_static_string<Length, CharT> >
{
};

template <size_t Length_l, typename CharT_l, size_t Length_r, typename CharT_r, class... S>
struct string_join_result<basic_static_string<Length_l, CharT_l>, basic_static_string<Length_r, CharT_r>, S...> :
    string_join_result<basic_static_string<Length_l + Length_r, string_join_char_t<CharT_l, CharT_r> >, S...>
{
};

template <size_t Length_l, typename CharT_l, size_t Length_r, typename CharT_r, class... S>
struct string_join_result<basic_static_string<Length_l, CharT_l> const, basic_static_string<Length_r, CharT_r>, S...> :
    string_join_result<basic_static_string<Length_l + Length_r, CharT_l> const, S...>
{
};

template <size_t Length_l, typename CharT_l, size_t Length_r, typename CharT_r, class... S>
struct string_join_result<basic_static_string<Length_l, CharT_l>, CharT_r[Length_r], S...> :
    string_join_result<basic_static_string<Length_l + Length_r - 1, string_join_char_t<CharT_l, CharT_r> >, S...>
{
};

template <size_t Length_l, typename CharT_l, size_t Length_r, typename CharT_r, class... S>
struct string_join_result<basic_static_string<Length_l, CharT_l> const, CharT_r[Length_r], S...> :
    string_join_result<basic_static_string<Length_l + Length_r - 1, CharT_l> const, S...>
{
};
#endif

template <size_t Length, typename CharT>
class basic_static_string<Length, CharT const>;

template <typename CharT>
class basic_static_string<0, CharT>;

template <typename CharT>
class basic_static_string<-1, CharT>;

template <typename CharT, size_t Length>
basic_static_string(CharT const (&)[Length]) -> basic_static_string<Length - 1, CharT>;
#endif

template <size_t Length>
using static_string = basic_static_string<Length, char>;
template <size_t Length>
using static_wstring = basic_static_string<Length, wchar_t>;
template <size_t Length>
using static_u8string = basic_static_string<Length, char8_t>;

template <size_t Length, typename CharT>
class basic_constant_string
{
    using buffer_type = CharT[Length];

  public:
    using size_type       = small_type<size_t, Length>;
    using difference_type = /*small_type<ptrdiff_t, Length>*/ ptrdiff_t;

    using value_type      = CharT;
    using pointer         = CharT*;
    using const_pointer   = CharT const*;
    using reference       = CharT&;
    using const_reference = CharT const&;

    using iterator       = detail::select_iterator<typename span<CharT>::iterator, pointer>;
    using const_iterator = detail::select_iterator<typename span<CharT const>::iterator, const_pointer>;

    buffer_type buffer_;

    constexpr basic_constant_string()
        : buffer_{}
    {
    }

    template <typename It>
    constexpr basic_constant_string(It first, It last)
    {
        assert(std::distance(first, last) == Length);
        std::copy(first, last, std::data(buffer_));
    }

    template <std::incrementable It>
    constexpr basic_constant_string(It source)
    {
        std::copy_n(source, Length, std::data(buffer_));
    }

    constexpr pointer data() noexcept
    {
        return std::data(buffer_);
    }

    constexpr const_pointer data() const noexcept
    {
        return std::data(buffer_);
    }

    constexpr integral_constant<size_type, Length> size() const noexcept
    {
        return {};
    }

    constexpr integral_constant<size_type, Length> length() const noexcept
    {
        return {};
    }

    static integral_constant<size_type, Length> max_size() noexcept
    {
        return {};
    }

    constexpr iterator begin() noexcept
    {
        if constexpr (std::is_pointer_v<iterator>)
            return data();
        else
            return span{buffer_}.begin();
    }

    constexpr const_iterator begin() const noexcept
    {
        if constexpr (std::is_pointer_v<const_iterator>)
            return data();
        else
            return span{buffer_}.begin();
    }

    constexpr iterator end() noexcept
    {
        if constexpr (std::is_pointer_v<iterator>)
            return data() + Length;
        else
            return span{buffer_}.end();
    }

    constexpr const_iterator end() const noexcept
    {
        if constexpr (std::is_pointer_v<const_iterator>)
            return data() + Length;
        else
            return span{buffer_}.end();
    }

#ifdef _MSC_VER
    // ReSharper disable CppInconsistentNaming

    constexpr pointer _Unchecked_begin() noexcept
    {
        return data();
    }

    constexpr const_pointer _Unchecked_begin() const noexcept
    {
        return data();
    }

    constexpr pointer _Unchecked_end() noexcept
    {
        return data() + Length;
    }

    constexpr const_pointer _Unchecked_end() const noexcept
    {
        return data() + Length;
    }

    // ReSharper restore CppInconsistentNaming
#endif
};

template <typename CharT, size_t Length>
basic_constant_string(CharT const (&)[Length]) -> basic_constant_string<Length - 1, CharT>;

template <size_t Length>
using constant_string = basic_constant_string<Length, char>;
template <size_t Length>
using constant_wstring = basic_constant_string<Length, wchar_t>;
template <size_t Length>
using constant_u8string = basic_constant_string<Length, char8_t>;

namespace detail
{
#if 0
template <class T>
struct is_static_string_impl : false_type
{
};

template <size_t L, typename Ch>
struct is_static_string_impl<basic_static_string<L, Ch>> : true_type
{
};

template <class T>
concept is_static_string = is_static_string_impl<T>::value;

template <class T>
struct is_constant_string_impl : false_type
{
};

template <size_t L, typename Ch>
struct is_constant_string_impl<basic_constant_string<L, Ch>> : true_type
{
};

template <class T>
concept is_constant_string = is_constant_string_impl<T>::value;
#else
template <class T>
concept is_static_string = requires(T str) {
    []<size_t L, typename Ch>(basic_static_string<L, Ch>) {
    }(str);
};
template <class T>
concept is_constant_string = requires(T str) {
    []<size_t L, typename Ch>(basic_constant_string<L, Ch>) {
    }(str);
};
#endif
template <class S>
concept has_max_size = requires { 1u + S::max_size(); };

template <class S>
concept known_max_size = has_max_size<S> || std::is_bounded_array_v<S>;

template <class T>
struct string_value : type_identity<typename T::value_type>
{
};

template <class T, size_t S>
struct string_value<T[S]> : type_identity<T>
{
};

template <class T>
struct string_max_size;

template <size_t L, typename C>
struct string_max_size<basic_static_string<L, C>> : integral_constant<size_t, L>
{
};

template <size_t L, typename C>
struct string_max_size<basic_constant_string<L, C>> : integral_constant<size_t, L>
{
};

template <class T, size_t S>
struct string_max_size<T[S]> : integral_constant<size_t, S - 1>
{
};

template <template <size_t L, typename C> class String, class S_l, class S_r>
using sized_string_join_result = String<
    string_max_size<S_l>::value + string_max_size<S_r>::value, //
    string_join_char_t<typename string_value<S_l>::type, typename string_value<S_r>::type>>;

template <class S_l, class S_r>
using static_string_join_result = sized_string_join_result<basic_static_string, S_l, S_r>;
template <class S_l, class S_r>
using constant_string_join_result = sized_string_join_result<basic_constant_string, S_l, S_r>;

template <typename T>
constexpr auto string_end(T const& str)
{
    using std::end;
    return end(str) - std::is_bounded_array_v<T>;
}

template <typename T>
constexpr void validate_raw_string(T const& str)
{
    if constexpr (std::is_bounded_array_v<T>)
        assert(*std::rbegin(str) == '\0');
}

template <typename T>
constexpr void validate_static_string(T const& str)
{
    if constexpr (is_static_string<T>)
        assert(str.size() == str.max_size());
}

template <typename S_l, typename S_r>
constexpr auto join_to_static_string(S_l const& str_l, S_r const& str_r) -> static_string_join_result<S_l, S_r>
{
    static_string_join_result<S_l, S_r> ret;

#ifdef BOOST_STATIC_STRING_HPP
    using std::begin;

    ret.append(begin(str_l), string_end(str_l));
    ret.append(begin(str_r), string_end(str_r));
#else
    ret.append(str_l);
    ret.append(str_r);
#endif
    return ret;
}

template <typename S_l, typename S_r>
constexpr auto join_to_constant_string(S_l const& str_l, S_r const& str_r) -> constant_string_join_result<S_l, S_r>
{
    validate_raw_string(str_r);
    validate_static_string(str_r);
    validate_raw_string(str_l);
    validate_static_string(str_l);

    using std::begin;
    using std::data;
    using std::end;

    constant_string_join_result<S_l, S_r> ret;
    auto dst_it = data(ret);
    dst_it      = std::copy(begin(str_l), string_end(str_l), dst_it);
    /*dst_it   =*/std::copy(begin(str_r), string_end(str_r), dst_it);
    return ret;
}

template <class L, class R>
constexpr bool equal_string_size(L const& str_l, R const& str_r)
{
    using std::size;
    return size(str_l) - std::is_bounded_array_v<L> == size(str_r) - std::is_bounded_array_v<R>;
}

template <typename S_l, typename S_r>
constexpr bool equal_string(S_l const& str_l, S_r const& str_r)
{
    validate_raw_string(str_l);
    validate_raw_string(str_r);

    using std::begin;
    using std::equal;

    return equal_string_size(str_l, str_r) && equal(begin(str_l), string_end(str_l), begin(str_r));
}
} // namespace detail

template <typename S_l, typename S_r>
constexpr detail::static_string_join_result<S_l, S_r> operator+(S_l const& str_l, S_r const& str_r) requires(
    (detail::is_static_string<S_l> || detail::is_static_string<S_r>) &&
#ifdef BOOST_STATIC_STRING_HPP
    (detail::is_constant_string<S_l> || detail::is_constant_string<S_r>)
#else
    (detail::known_max_size<S_l> || detail::known_max_size<S_r>)
#endif
)
{
    return detail::join_to_static_string(str_l, str_r);
}

template <typename S_l, typename S_r>
constexpr auto operator+(S_l const& str_l, S_r const& str_r) -> detail::constant_string_join_result<S_l, S_r> requires(
    (detail::is_constant_string<S_l> || detail::is_constant_string<S_r>) && //
    (std::is_bounded_array_v<S_l> || std::is_bounded_array_v<S_r>))
{
    return detail::join_to_constant_string(str_l, str_r);
}

template <typename S_l, typename S_r>
constexpr bool operator==(S_l const& str_l, S_r const& str_r) requires(
    (detail::has_max_size<S_l> || detail::has_max_size<S_r>) && //
    (detail::known_max_size<S_l> || detail::known_max_size<S_r>))
{
    return detail::equal_string(str_l, str_r);
}

inline namespace literals
{
template <basic_constant_string Str>
constexpr auto operator"" _cs() -> std::remove_const_t<decltype(Str)>
{
    return Str;
}
} // namespace literals
} // namespace fd
