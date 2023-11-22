#pragma once

#if 1
#include "container/array.h"
#include "iterator/unwrap.h"
#else
#define BOOST_STATIC_STRING_STANDALONE
#include <boost/static_string.hpp>
#endif

namespace fd
{
#ifdef BOOST_STATIC_STRING_HPP
using boost::static_strings::basic_static_string;
#else

#if defined(_MSC_VER)
#if _ITERATOR_DEBUG_LEVEL != 0
#define STATIC_STRING_DEBUG_ITER
#endif
#elif defined(_DEBUG)
#define STATIC_STRING_DEBUG
#endif

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
#ifdef STATIC_STRING_DEBUG_ITER
    using iterator       = typename span<CharT>::iterator;
    using const_iterator = typename span<CharT const>::iterator;
#else
    using iterator       = pointer;
    using const_iterator = const_pointer;
#endif

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

    constexpr size_type length() const noexcept
    {
        return size_;
    }

    constexpr size_type max_size() const noexcept
    {
        (void)this;
        return Length;
    }

#ifdef STATIC_STRING_DEBUG_ITER
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
#else
    constexpr iterator begin() noexcept
    {
        return data();
    }

    constexpr const_iterator begin() const noexcept
    {
        return data();
    }

    constexpr iterator end() noexcept
    {
        return data() + size_;
    }

    constexpr const_iterator end() const noexcept
    {
        return data() + size_;
    }
#endif

#ifdef _MSC_VER
    // ReSharper disable CppInconsistentNaming

    constexpr pointer _Unchecked_begin() noexcept
    {
        return ubegin(buffer_);
    }

    constexpr pointer _Unchecked_begin() const noexcept
    {
        return ubegin(buffer_);
    }

    constexpr pointer _Unchecked_end() noexcept
    {
        return _Unchecked_begin() + size_;
    }

    constexpr pointer _Unchecked_end() const noexcept
    {
        return _Unchecked_begin() + size_;
    }

    // ReSharper restore CppInconsistentNaming
#endif
};

template <typename CharT, size_t Length>
basic_static_string(CharT const (&)[Length]) -> basic_static_string<Length - 1, CharT>;
#undef STATIC_STRING_DEBUG_ITER
#endif

template <size_t Length>
using static_string = basic_static_string<Length, char>;
template <size_t Length>
using static_wstring = basic_static_string<Length, wchar_t>;
template <size_t Length>
using static_u8string = basic_static_string<Length, char8_t>;
} // namespace fd
