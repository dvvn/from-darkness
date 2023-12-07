#pragma once

#include "concepts.h"
#if 1
#include "container/array.h"
#else
#define BOOST_STATIC_STRING_STANDALONE
#include <boost/static_string.hpp>
#endif

#include <cassert>

namespace fd
{
#ifdef BOOST_STATIC_STRING_HPP
using boost::static_strings::basic_static_string;
#else

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
        auto const buffer_begin = std::begin(buffer_);
        auto const buffer_last  = std::copy(first, last, buffer_begin);
        size_                   = static_cast<size_type>(std::distance(buffer_begin, buffer_last));
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
    using size_type = std::conditional_t<
        Length <= UINT8_MAX, uint8_t, //
        std::conditional_t<
            Length <= UINT16_MAX, uint16_t, //
            std::conditional_t<
                Length <= UINT32_MAX, uint32_t, //
                uint64_t> > >;

    using difference_type = std::make_signed_t<size_type>;

    using value_type      = CharT;
    using pointer         = CharT*;
    using const_pointer   = CharT const*;
    using reference       = CharT&;
    using const_reference = CharT const&;

    using iterator       = typename span<CharT>::iterator;
    using const_iterator = typename span<CharT const>::iterator;

    buffer_type buffer_;
    size_type size_;

    constexpr basic_static_string()
        : buffer_{}
        , size_{0}
    {
    }

    /*template <typename CharT2 = CharT>
    constexpr basic_static_string(basic_static_string<Length, CharT2> const& other)
    {
        pre_construct();
        assign_internal(other.begin(), other.end());
    }*/

    /*template <typename It>
    constexpr basic_static_string(It first, It last) requires(std::convertible_to<std::iter_value_t<It>, value_type>)
    {
        pre_construct();
        assign_internal(first, last);
    }*/

    template <typename It>
    constexpr basic_static_string(It first, It last)
    {
        pre_construct();
        assign_internal(first, last);
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

    static size_type max_size() noexcept
    {
        return Length;
    }

    constexpr iterator begin() noexcept
    {
        return span{data(), size_}.begin();
    }

    constexpr const_iterator begin() const noexcept
    {
        return span{data(), size_}.begin();
    }

    constexpr iterator end() noexcept
    {
        return span{data(), size_}.end();
    }

    constexpr const_iterator end() const noexcept
    {
        return span{data(), size_}.end();
    }

#ifdef _MSC_VER
    // ReSharper disable CppInconsistentNaming

    constexpr pointer _Unchecked_begin() noexcept
    {
        return (buffer_);
    }

    constexpr const_pointer _Unchecked_begin() const noexcept
    {
        return (buffer_);
    }

    constexpr pointer _Unchecked_end() noexcept
    {
        return _Unchecked_begin() + size_;
    }

    constexpr const_pointer _Unchecked_end() const noexcept
    {
        return _Unchecked_begin() + size_;
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
        size_ += extra_size;
        return *this;
    }

    template <typename It, std::integral N>
    constexpr basic_static_string& append(It first, N count)
    {
        assert(size_ + count <= Length);
        if constexpr (std::random_access_iterator<It>)
            std::copy(first, first + count, buffer_ + size_);
        else
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
            if (rng[rng_size - 1] == '\0')
                --rng_size;
        }

        return append(data(rng), rng_size);
    }
};

// template <typename T>
// decltype(auto) uendstr(T&& str)
//{
//     decltype(auto) ret = uend(std::forward<T>(str));
//     if constexpr (std::is_bounded_array_v<std::remove_cv_t<T> >)
//     {
//         auto back_it = std::prev(ret);
//         if (*back_it == '\0')
//             ret = std::move(back_it);
//     }
//     return std::forward<decltype(ret)>(ret);
// }

template <typename CharT_l, typename CharT_r>
struct string_join_char :
    std::conditional<
        sizeof(CharT_l) >= sizeof(CharT_r), //
        CharT_l, CharT_r>
{
};

template <typename CharT_l, typename CharT_r>
using string_join_char_t = typename string_join_char<CharT_l, CharT_r>::type;

template <class Result, class... S>
struct string_join_result;

template <size_t Length, typename CharT, class... S>
struct string_join_result<CharT[Length], S...> :
    string_join_result<
        basic_static_string<Length - 1, std::remove_const_t<CharT> >, //
        S...>
{
};

template <size_t Length, typename CharT>
struct string_join_result<basic_static_string<Length, CharT> > : std::type_identity<basic_static_string<Length, CharT> >
{
};

template <size_t Length, typename CharT>
struct string_join_result<basic_static_string<Length, CharT> const> : std::type_identity<basic_static_string<Length, CharT> >
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

template <size_t Length, typename CharT>
class basic_static_string<Length, CharT const>;

template <typename CharT>
class basic_static_string<0, CharT>;

template <typename CharT>
class basic_static_string<-1, CharT>;

#if 0
template <size_t Length, typename CharT, typename Other>
constexpr bool operator==(basic_static_string<Length, CharT> const& left, Other const& right) requires(std::same_as<std::iter_value_t<Other>, CharT>)
{
     return std::equal(ubegin(left), uend(left), ubegin(right), uendstr(right));
}
#endif

template <size_t Length_l, typename CharT_l, typename CharT_r, size_t Length_r>
constexpr auto operator+(basic_static_string<Length_l, CharT_l> const& str_l, basic_static_string<Length_r, CharT_r> const& str_r)
    -> basic_static_string<Length_l + Length_r, string_join_char_t<CharT_l, CharT_r> >
{
    basic_static_string<Length_l + Length_r, string_join_char_t<CharT_l, CharT_r> > ret;
    ret.append(str_l);
    ret.append(str_r);
    return ret;
}

template <size_t Length_l, typename CharT_l, typename CharT_r, size_t Length_r>
constexpr auto operator+(basic_static_string<Length_l, CharT_l> const& str_l, CharT_r const (&str_r)[Length_r])
    -> basic_static_string<Length_l + Length_r - 1, string_join_char_t<CharT_l, CharT_r> >
{
    basic_static_string<Length_l + Length_r - 1, string_join_char_t<CharT_l, CharT_r> > ret;
    ret.append(str_l);
    ret.append(str_r, Length_r - 1);
    return ret;
}

template <typename CharT_l, size_t Length_l, size_t Length_r, typename CharT_r>
constexpr auto operator+(CharT_l const (&str_l)[Length_l], basic_static_string<Length_r, CharT_r> const& str_r)
    -> basic_static_string<Length_l - 1 + Length_r, string_join_char_t<CharT_l, CharT_r> >
{
    basic_static_string<Length_l - 1 + Length_r, string_join_char_t<CharT_l, CharT_r> > ret;
    ret.append(str_l, Length_l - 1);
    ret.append(str_r);
    return ret;
}

template <typename CharT, size_t Length>
basic_static_string(CharT const (&)[Length]) -> basic_static_string<Length - 1, CharT>;
#endif

template <size_t Length>
using static_string = basic_static_string<Length, char>;
template <size_t Length>
using static_wstring = basic_static_string<Length, wchar_t>;
template <size_t Length>
using static_u8string = basic_static_string<Length, char8_t>;
} // namespace fd
