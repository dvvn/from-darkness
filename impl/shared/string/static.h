#pragma once

#if 1
#include "container/array.h"
#include "iterator/unwrap.h"
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
        auto const buffer_end   = std::copy(first, last, buffer_begin);
        size_                   = static_cast<size_type>(std::distance(buffer_begin, buffer_end));
#ifdef _DEBUG
        if (size_ < old_size)
            std::fill(buffer_end, std::end(buffer_), static_cast<CharT>('\0'));
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

    constexpr size_type max_size() const noexcept
    {
        (void)this;
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
};

template <size_t Length, typename CharT, typename Other>
constexpr bool operator==(basic_static_string<Length, CharT> const& left, Other const& right) requires(std::same_as<std::iter_value_t<Other>, CharT>)
{
    using std::equal;
    auto right_end = uend(right);
    if constexpr (std::is_bounded_array_v<Other>)
        if (*std::prev(right_end) == '\0')
            --right_end;
    return equal(ubegin(left), uend(left), ubegin(right), right_end);
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
