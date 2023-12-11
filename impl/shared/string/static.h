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
    It, Ptr>;
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
        auto const buffer_first = std::data(buffer_);
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

template <size_t Length_l, typename CharT_l, size_t Length_r, typename CharT_r>
constexpr bool operator==(basic_static_string<Length_l, CharT_l> const& str_l, basic_static_string<Length_r, CharT_r> const& str_r)
{
    auto const l_length = str_l.length();
    if (l_length == str_r.length())
    {
        auto const l_data = str_l.data();
        return std::equal(l_data, l_data + l_length, str_r.data());
    }
    return false;
}

template <size_t Length_l, typename CharT_l, typename CharT_r, size_t Length_r>
constexpr bool operator==(basic_static_string<Length_l, CharT_l> const& str_l, CharT_r const (&str_r)[Length_r])
{
    constexpr auto r_length = Length_r - 1;
    assert(str_r[r_length] == '\0');

    if constexpr (Length_l >= r_length)
    {
        auto const l_length = str_l.length();
        if (l_length == r_length)
        {
            auto const l_data = str_l.data();
            return std::equal(l_data, l_data + r_length, str_r);
        }
    }
    return false;
}

template <size_t Length_l, typename CharT_l, size_t Length_r, typename CharT_r>
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
    constexpr auto r_length = Length_r - 1;
    assert(str_r[r_length] == '\0');
    basic_static_string<Length_l + r_length, string_join_char_t<CharT_l, CharT_r> > ret;
    ret.append(str_l);
    ret.append(str_r, r_length);
    return ret;
}

template <typename CharT_l, size_t Length_l, size_t Length_r, typename CharT_r>
constexpr auto operator+(CharT_l const (&str_l)[Length_l], basic_static_string<Length_r, CharT_r> const& str_r)
    -> basic_static_string<Length_l - 1 + Length_r, string_join_char_t<CharT_l, CharT_r> >
{
    constexpr auto l_length = Length_l - 1;
    assert(str_r[l_length] == '\0');
    basic_static_string<l_length + Length_r, string_join_char_t<CharT_l, CharT_r> > ret;
    ret.append(str_l, l_length);
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
        if constexpr (std::random_access_iterator<It>)
            std::copy(source, source + Length, std::data(buffer_));
        else
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

template <size_t Length_l, typename CharT_l, typename CharT_r, size_t Length_r>
constexpr auto operator+(basic_constant_string<Length_l, CharT_l> const& str_l, CharT_r const (&str_r)[Length_r])
    -> basic_constant_string<Length_l + Length_r - 1, string_join_char_t<CharT_l, CharT_r> >
{
    constexpr auto r_length = Length_r - 1;
    assert(str_r[r_length] == '\0');
    basic_static_string<Length_l + r_length, string_join_char_t<CharT_l, CharT_r> > ret;
    auto dst_it = ret.data();
    dst_it      = std::copy(str_l.data(), str_l.data() + str_l.size(), dst_it);
    /*dst_it   =*/std::copy(str_r, str_r + r_length, dst_it);
    return ret;
}

template <size_t Length_l, typename CharT_l, size_t Length_r, typename CharT_r>
constexpr bool operator==(basic_constant_string<Length_l, CharT_l> const& str_l, basic_constant_string<Length_r, CharT_r> const& str_r)
{
    if constexpr (Length_l == Length_r)
    {
        auto const l_data = str_l.data();
        return std::equal(l_data, l_data + Length_l, str_r.data());
    }
    return false;
}

template <size_t Length_l, typename CharT_l, typename CharT_r, size_t Length_r>
constexpr bool operator==(basic_constant_string<Length_l, CharT_l> const& str_l, CharT_r const (&str_r)[Length_r])
{
    constexpr auto r_length = Length_r - 1;
    assert(str_r[r_length] == '\0');

    if constexpr (Length_l == r_length)
    {
        auto const l_data = str_l.data();
        return std::equal(l_data, l_data + r_length, str_r);
    }
    return false;
}

template <typename CharT, size_t Length>
basic_constant_string(CharT const (&)[Length]) -> basic_constant_string<Length - 1, CharT>;

template <size_t Length>
using constant_string = basic_constant_string<Length, char>;
template <size_t Length>
using constant_wstring = basic_constant_string<Length, wchar_t>;
template <size_t Length>
using constant_u8string = basic_constant_string<Length, char8_t>;
} // namespace fd
