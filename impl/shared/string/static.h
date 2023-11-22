#pragma once

#if 1
#include "container/array.h"
#include "iterator/unwrap.h"
#include "string/view.h"
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
    using helper_string     = basic_string_view<CharT>;
    using helper_span       = span<CharT>;
    using helper_span_const = span<CharT const>;

    using buffer_type = CharT[Length];

    template <typename... Args>
    constexpr void assign_internal(Args&&... args) noexcept(noexcept(helper_string{std::forward<Args>(args)...}))
    {
#ifdef _DEBUG
        auto const old_size = size_;
#endif
        helper_string const helper{std::forward<Args>(args)...};
        auto const buffer_end = std::copy(helper.begin(), helper.end(), std::begin(buffer_));
        size_                 = static_cast<size_type>(helper.size());
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
    using iterator       = typename helper_span::iterator;
    using const_iterator = typename helper_span_const::iterator;
#else
    using iterator       = pointer;
    using const_iterator = const_pointer;
#endif
    using traits_type = typename helper_string::traits_type;

    buffer_type buffer_;
    size_type size_;

    constexpr basic_static_string()
        : buffer_{}
        , size_{0}
    {
    }

    template <typename... Args>
    constexpr basic_static_string(Args&&... args) requires(sizeof...(Args) > 1 && std::constructible_from<helper_string, Args && ...>)
    {
        pre_construct();
        assign_internal(std::forward<Args>(args)...);
    }

    constexpr basic_static_string(CharT const* source)
    {
        pre_construct();
        assign_internal(source, Length);
    }

    template <typename... Args>
    constexpr basic_static_string& assign(Args&&... args) requires(std::constructible_from<helper_string, Args && ...>)
    {
        pre_construct();
        assign_internal(std::forward<Args>(args)...);
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

    constexpr size_type max_size() const noexcept
    {
        (void)this;
        return Length;
    }

#ifdef STATIC_STRING_DEBUG_ITER
    constexpr iterator begin() noexcept
    {
        return helper_span{data(), size_}.begin();
    }

    constexpr const_iterator begin() const noexcept
    {
        return helper_span_const{data(), size_}.begin();
    }

    constexpr iterator end() noexcept
    {
        return helper_span{data(), size_}.end();
    }

    constexpr const_iterator end() const noexcept
    {
        return helper_span_const{data(), size_}.end();
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
