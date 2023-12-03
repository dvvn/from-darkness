#pragma once

#include "tier0/core.h"
#include "tier1/concepts.h"
#if 1
#include "tier0/iterator/unwrap.h"
#include "tier1/container/array.h"
#else
#define BOOST_STATIC_STRING_STANDALONE
#include <boost/static_string.hpp>
#endif

#include <cassert>

namespace FD_TIER(1)
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
                uint64_t>>>;

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
        return ubegin(buffer_);
    }

    constexpr const_pointer _Unchecked_begin() const noexcept
    {
        return ubegin(buffer_);
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
    {
        auto rng_last = uend(rng);
        if constexpr (std::is_bounded_array_v<Rng>)
            if (*std::prev(rng_last) == '\0')
                --rng_last;
        return append(ubegin(rng), rng_last);
    }
};

template <size_t Length, typename CharT>
class basic_static_string<Length, CharT const>;

template <typename CharT>
class basic_static_string<0, CharT>;

template <typename CharT>
class basic_static_string<-1, CharT>;

template <size_t Length, typename CharT, typename Other>
constexpr bool operator==(basic_static_string<Length, CharT> const& left, Other const& right) requires(std::same_as<std::iter_value_t<Other>, CharT>)
{
    auto right_last = uend(right);
    if constexpr (std::is_bounded_array_v<Other>)
        if (*std::prev(right_last) == '\0')
            --right_last;
    return std::equal(ubegin(left), uend(left), ubegin(right), right_last);
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

template <class Result, class... S>
struct to_static_string_result;

template <size_t Length, typename CharT, class... S>
struct to_static_string_result<CharT[Length], S...> :
    to_static_string_result<
        basic_static_string<Length - 1, std::remove_const_t<CharT>>, //
        S...>
{
};

template <size_t Length, typename CharT>
struct to_static_string_result<basic_static_string<Length, CharT>> : std::type_identity<basic_static_string<Length, CharT>>
{
};

template <size_t Length, typename CharT>
struct to_static_string_result<basic_static_string<Length, CharT> const> : std::type_identity<basic_static_string<Length, CharT>>
{
};

template <size_t Length_l, typename CharT_l, size_t Length_r, typename CharT_r, class... S>
struct to_static_string_result<basic_static_string<Length_l, CharT_l>, basic_static_string<Length_r, CharT_r>, S...> :
    to_static_string_result<
        basic_static_string<
            Length_l + Length_r,
            std::conditional_t<
                sizeof(CharT_l) >= sizeof(CharT_r), //
                CharT_l, CharT_r>>,
        S...>
{
};

template <size_t Length_l, typename CharT_l, size_t Length_r, typename CharT_r, class... S>
struct to_static_string_result<basic_static_string<Length_l, CharT_l> const, basic_static_string<Length_r, CharT_r>, S...> :
    to_static_string_result<
        basic_static_string<
            Length_l + Length_r, //
            CharT_l> const,
        S...>
{
};

template <size_t Length_l, typename CharT_l, size_t Length_r, typename CharT_r, class... S>
struct to_static_string_result<basic_static_string<Length_l, CharT_l>, CharT_r[Length_r], S...> :
    to_static_string_result<
        basic_static_string<
            Length_l + Length_r - 1,
            std::conditional_t<
                sizeof(CharT_l) >= sizeof(CharT_r), //
                CharT_l, std::remove_const_t<CharT_r>>>,
        S...>
{
};

template <size_t Length_l, typename CharT_l, size_t Length_r, typename CharT_r, class... S>
struct to_static_string_result<basic_static_string<Length_l, CharT_l> const, CharT_r[Length_r], S...> :
    to_static_string_result<
        basic_static_string<
            Length_l + Length_r - 1, //
            CharT_l> const,
        S...>
{
};

template <typename CharT = void>
inline constexpr auto to_static_string = []<typename... Str>(Str const&... str) ->
    typename to_static_string_result<basic_static_string<0, CharT> const, Str...>::type {
        typename to_static_string_result<basic_static_string<0, CharT> const, Str...>::type out_str;
        (out_str.append(str), ...);
        return out_str;
    };

template <>
inline constexpr auto to_static_string<void> = []<typename... Str>(Str const&... str) -> typename to_static_string_result<Str...>::type {
    typename to_static_string_result<Str...>::type out_str;
    (out_str.append(str), ...);
    return out_str;
};

template <typename CharT_out = void, typename... Str>
constexpr complete auto to_string(Str const&... str) ->
    typename to_static_string_result<basic_static_string<0, std::conditional_t<std::is_void_v<CharT_out>, char, CharT_out>>, Str...>::type
{
    return to_static_string<CharT_out>(str...);
}
} // namespace FD_TIER(1)
