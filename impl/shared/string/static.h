#pragma once
#include "container/span.h"
#include "string/view.h"
#include "type_traits/conditional.h"
#include "type_traits/integral_constant.h"
#include "type_traits/small_type.h"

#include <algorithm>
#include <cassert>

namespace fd
{
template <typename CharT_l, typename CharT_r>
struct string_join_char : conditional<sizeof(CharT_l) >= sizeof(CharT_r), CharT_l, CharT_r>
{
};

template <typename CharT_l, typename CharT_r>
using string_join_char_t = typename string_join_char<CharT_l, CharT_r>::type;

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

template <typename CharT, size_t Length, class Config>
class basic_static_string_full
{
    static constexpr size_t buffer_size = Config::null_terminated ? Length + 1 : Length;

    using iterator_holder       = span<CharT>;
    using const_iterator_holder = basic_string_view<CharT> /*span<CharT const>*/;

    iterator_holder make_iterator_holder()
    {
        return {data(), size_};
    }

    const_iterator_holder make_iterator_holder() const
    {
        return {data(), size_};
    }

  public:
    using value_type      = CharT;
    using pointer         = CharT*;
    using const_pointer   = CharT const*;
    using reference       = CharT&;
    using const_reference = CharT const&;

    using iterator       = select_iterator<typename iterator_holder::iterator, pointer>;
    using const_iterator = select_iterator<typename const_iterator_holder::iterator, const_pointer>;

    using size_type       = small_type<size_t, buffer_size>;
    using difference_type = /*small_type<ptrdiff_t, buffer_size>*/ ptrdiff_t;

    using config_type = Config;

    template <class Config2>
    using rebind_config = basic_static_string_full<CharT, Length, Config2>;

    template <class Config2>
    constexpr basic_static_string_full<CharT, Length, Config2> set_config(Config2 config = {}) const
    {
        return {*this, config};
    }

  private:
    using buffer_type = CharT[buffer_size];

    using max_size_constant = integral_constant<size_type, Length>;
    using size_type_stored  = conditional_t<Config::dynamic, size_type, max_size_constant>;

  public:
    [[no_unique_address]] //
    config_type config_;
    buffer_type buffer_;
    [[no_unique_address]] //
    size_type_stored size_;

  private:
    constexpr void write_null_terminator()
    {
        if constexpr (Config::null_terminated)
            if constexpr (Config::dynamic)
                buffer_[size_] = '\0';
            else
                buffer_[Length] = '\0';
    }

    /*constexpr void fill_unused(pointer it)
    {
        std::fill_n(it, buffer_size - size_, static_cast<CharT>('\0'));
    }

    constexpr void fill_unused()
    {
        std::fill(buffer_ + size_, buffer_ + buffer_size, static_cast<CharT>('\0'));
    }*/

  public:
    constexpr basic_static_string_full(config_type config = {})
        : config_{config}
        , buffer_{}
        , size_{}
    {
    }

    template <typename It>
    constexpr basic_static_string_full(It first, It last)
        : basic_static_string_full{}
    {
        assign(first, last);
    }

    template <typename CharT2 = CharT, size_t Length2 = Length + 1>
    constexpr basic_static_string_full(CharT2 const (&arr)[Length2], Config config = {})
        : basic_static_string_full{config}
    {
        assign_range(arr);
    }

    template <typename ConfigOld>
    constexpr basic_static_string_full(basic_static_string_full<CharT, Length, ConfigOld> const& str, Config config)
        : basic_static_string_full{config}
    {
        assign_range(str);
    }

    /*constexpr basic_static_string_full(value_type const (&arr)[Length]) requires(Config::null_terminated)
        : basic_static_string_full{}
    {
        assign_range(arr);
    }*/

    template <std::incrementable It>
    constexpr basic_static_string_full(It source)
        : basic_static_string_full{}
    {
        assign(source, Length);
    }

    constexpr void push_back(value_type chr) requires(Config::dynamic)
    {
        assert(size_ < Length);
        buffer_[size_] = chr;
        ++size_;
        write_null_terminator();
    }

  private:
    template <typename It>
    constexpr size_type append(size_type buff_size, It first, It last)
    {
        auto const extra_size = std::distance(first, last);
        assert(buff_size + extra_size <= Length);
        std::copy(first, last, buffer_ + buff_size);
        write_null_terminator();
        return static_cast<size_type>(extra_size);
    }

    template <typename It>
    constexpr void append(size_type size, It first, size_type count)
    {
        assert(size + count <= Length);
        std::copy_n(first, count, buffer_ + size);
        write_null_terminator();
    }

    template <typename Rng>
    constexpr size_type append_range(size_type buff_size, Rng const& rng)
    {
        if constexpr (std::ranges::forward_range<Rng> || std::ranges::sized_range<Rng>)
        {
            using std::data;
            using std::size;
            auto rng_size = size(rng);
            if constexpr (std::is_bounded_array_v<Rng>)
            {
                --rng_size;
                assert(rng[rng_size] == '\0');
            }
            append(buff_size, data(rng), rng_size);
            return static_cast<size_type>(rng_size);
        }
        else
        {
            using std::begin;
            using std::end;
            return append(buff_size, begin(rng), end(rng));
        }
    }

  public:
    template <typename It>
    constexpr basic_static_string_full& append(It first, It last) requires(Config::dynamic)
    {
        size_ += append(size_, first, last);
        return *this;
    }

    template <typename It>
    constexpr basic_static_string_full& append(It first, size_type count) requires(Config::dynamic)
    {
        append(size_, first, count);
        size_ += count;
        return *this;
    }

    template <typename Rng>
    constexpr basic_static_string_full& append_range(Rng const& rng) requires(Config::dynamic)
    {
        size_ += append_range(size_, rng);
        return *this;
    }

    template <typename It>
    constexpr basic_static_string_full& assign(It first, It last)
    {
        if constexpr (Config::dynamic)
            assert(std::distance(first, last) <= Length);
        else
            assert(std::distance(first, last) == Length);

        auto const buffer_first = std::data(buffer_);
        auto const buffer_last  = std::copy(first, last, buffer_first);
        if constexpr (Config::dynamic)
        {
            size_ = static_cast<size_type>(std::distance(buffer_first, buffer_last));
            // #ifdef _DEBUG
            //  fill_unused(buffer_last);
            //  return *this;
            // #endif
        }
        write_null_terminator();
        return *this;
    }

    template <typename It>
    constexpr basic_static_string_full& assign(It first, size_type length)
    {
        if constexpr (Config::dynamic)
            assert(length <= Length);
        else
            assert(length == Length);

        auto const buffer_first = std::data(buffer_);
        auto const buffer_last  = std::copy_n(first, length, buffer_first);
        if constexpr (Config::dynamic)
        {
            size_ = length;
            // #ifdef _DEBUG
            //   fill_unused(buffer_last);
            //   return *this;
            // #endif
        }
        write_null_terminator();
        return *this;
    }

    template <typename Rng>
    constexpr basic_static_string_full& assign_range(Rng const& rng)
    {
        if constexpr (Config::dynamic)
        {
            assert(size_ == 0);
            append_range(rng);
            // #ifdef _DEBUG
            //  fill_unused();
            // #endif
        }
        else
        {
            auto const rng_size = append_range(0, rng);
            assert(rng_size == size_);
        }

        return *this;
    }

    template <typename Rng, typename Rng2>
    constexpr basic_static_string_full& assign_range(Rng const& rng, Rng2 const& rng2)
    {
        if constexpr (Config::dynamic)
        {
            assert(size_ == 0);
            append_range(rng);
            append_range(rng2);
            // #ifdef _DEBUG
            //  fill_unused();
            // #endif
        }
        else
        {
            auto rng_size  = append_range(0, rng);
            auto rng2_size = append_range(rng_size, rng2);
            assert(rng_size + rng2_size == size_);
        }
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

    constexpr size_type_stored size() const noexcept
    {
        return size_;
    }

    constexpr size_type_stored length() const noexcept
    {
        return size_;
    }

    static max_size_constant max_size() noexcept
    {
        return {};
    }

    constexpr bool empty() const noexcept
    {
        if constexpr (Config::dynamic)
            return size_ == 0;
        else
            return false;
    }

    constexpr iterator begin() noexcept
    {
        if constexpr (std::is_pointer_v<iterator>)
            return data();
        else
            return make_iterator_holder().begin();
    }

    constexpr const_iterator begin() const noexcept
    {
        if constexpr (std::is_pointer_v<const_iterator>)
            return data();
        else
            return make_iterator_holder().begin();
    }

    constexpr iterator end() noexcept
    {
        if constexpr (std::is_pointer_v<iterator>)
            return data() + size_;
        else
            return make_iterator_holder().end();
    }

    constexpr const_iterator end() const noexcept
    {
        if constexpr (std::is_pointer_v<const_iterator>)
            return data() + size_;
        else
            return make_iterator_holder().end();
    }

    template <typename CharT_2, size_t Length_2>
    constexpr bool operator==(CharT_2 const (&other)[Length_2]) const
    {
        using std::equal;
        constexpr auto other_size = Length_2 - 1;
        assert(other[other_size] == '\0');
        return size_ == other_size && equal(other, other + other_size, data());
    }

  private:
    template <class Other>
    constexpr bool equal_helper(Other const& other) const noexcept
    {
        auto const other_size = other.size();
        if (other_size != size())
            return false;
        auto const other_data = other.data();
        using std::equal;
        if (!equal(other_data, other_data + other_size, data()))
            return false;
        return true;
    }

  public:
    template <typename CharT_2, size_t Length_2, class Config2>
    constexpr bool operator==(basic_static_string_full<CharT_2, Length_2, Config2> const& other) const
    {
        return equal_helper(other);
    }

    template <typename CharT_2>
    constexpr bool operator==(basic_string_view<CharT_2> const other) const
    {
        return equal_helper(other);
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
};

template <typename CharT, class Config>
class basic_static_string_full<CharT, 0, Config>;

template <bool NullTerminated, bool Dynamic>
struct basic_static_string_config
{
    static constexpr bool null_terminated = NullTerminated;
    static constexpr bool dynamic         = Dynamic;
};

#ifdef _DEBUG
// struct for better read
#define STATIC_STRING_CONFIG(_NAME_, ...)                   \
    struct _NAME_ : basic_static_string_config<__VA_ARGS__> \
    {                                                       \
    };
#else
using _NAME_ = basic_static_string_config < __VA_ARGS__;
#endif

STATIC_STRING_CONFIG(static_string_config, false, true)
STATIC_STRING_CONFIG(static_cstring_config, true, true)
STATIC_STRING_CONFIG(constant_string_config, false, false)
STATIC_STRING_CONFIG(constant_cstring_config, true, false)
#undef STATIC_STRING_CONFIG

template <typename S_l, typename S_r>
struct static_string_join_result;

template <typename S_l, typename S_r>
using static_string_join_result_t = typename static_string_join_result<S_l, S_r>::type;

template <typename CharT_l, size_t Length_l, typename Config_l, typename CharT_r, size_t Length_r, typename Config_r>
struct static_string_join_result<basic_static_string_full<CharT_l, Length_l, Config_l>, basic_static_string_full<CharT_r, Length_r, Config_r>> :
    type_identity<basic_static_string_full< //
        conditional_t<sizeof(CharT_l) >= sizeof(CharT_r), CharT_l, CharT_r>, Length_l + Length_r,
        basic_static_string_config<Config_l::null_terminated, Config_l::dynamic || Config_r::dynamic>>>
{
};

template <typename CharT_l, size_t Length_l, typename Config, typename CharT_r, size_t Length_r>
struct static_string_join_result<basic_static_string_full<CharT_l, Length_l, Config>, CharT_r[Length_r]> :
    type_identity<basic_static_string_full< //
        conditional_t<sizeof(CharT_l) >= sizeof(CharT_r), CharT_l, CharT_r>, Length_l + Length_r - 1, Config>>
{
};

template <typename CharT_l, size_t Length_l, typename Config_l, typename CharT_r, size_t Length_r, typename Config_r>
constexpr auto operator+(basic_static_string_full<CharT_l, Length_l, Config_l> const& str_l, basic_static_string_full<CharT_r, Length_r, Config_r> const& str_r)
    -> static_string_join_result_t<basic_static_string_full<CharT_l, Length_l, Config_l>, basic_static_string_full<CharT_r, Length_r, Config_r>>
{
    static_string_join_result_t<basic_static_string_full<CharT_l, Length_l, Config_l>, basic_static_string_full<CharT_r, Length_r, Config_r>> str_out;
    str_out.assign_range(str_l, str_r);
    return str_out;
}

template <typename CharT_l, size_t Length_l, typename Config_l, typename CharT_r, size_t Length_r>
constexpr auto operator+(basic_static_string_full<CharT_l, Length_l, Config_l> const& str_l, CharT_r const (&str_r)[Length_r])
    -> static_string_join_result_t<basic_static_string_full<CharT_l, Length_l, Config_l>, CharT_r[Length_r]>
{
    static_string_join_result_t<basic_static_string_full<CharT_l, Length_l, Config_l>, CharT_r[Length_r]> str_out;
    str_out.assign_range(str_l, str_r);
    return str_out;
}

template <typename CharT_l, size_t Length_l, typename CharT_r, size_t Length_r, typename Config_r>
constexpr auto operator+(CharT_l const (&str_l)[Length_l], basic_static_string_full<CharT_r, Length_r, Config_r> const& str_r)
    -> static_string_join_result_t<basic_static_string_full<CharT_r, Length_r, Config_r>, CharT_l[Length_l]>
{
    static_string_join_result_t<basic_static_string_full<CharT_r, Length_r, Config_r>, CharT_l[Length_l]> str_out;
    str_out.assign_range(str_l, str_r);
    return str_out;
}

template <typename CharT, size_t Length>
basic_static_string_full(CharT const (&)[Length]) -> basic_static_string_full<CharT, Length - 1, constant_cstring_config>;
template <typename CharT, size_t Length, typename Config>
basic_static_string_full(CharT const (&)[Length], Config) -> basic_static_string_full<CharT, Length - 1, Config>;
} // namespace detail

#define STATIC_STRING_SPEC_1(_NAME_, _PREFIX_)                                                                                                     \
    template <typename CharT, size_t Length>                                                                                                       \
    class basic_##_NAME_##_##_PREFIX_##string : public detail::basic_static_string_full<CharT, Length, detail::_NAME_##_##_PREFIX_##string_config> \
    {                                                                                                                                              \
      public:                                                                                                                                      \
        using detail::basic_static_string_full<CharT, Length, detail::_NAME_##_##_PREFIX_##string_config>::basic_static_string_full;               \
    };                                                                                                                                             \
    template <typename CharT, size_t Length>                                                                                                       \
    basic_##_NAME_##_##_PREFIX_##string(CharT const(&)[Length])->basic_##_NAME_##_##_PREFIX_##string<CharT, Length - 1>;
#define STATIC_STRING_SPEC_2(_NAME_, _PREFIX_)                 \
    STATIC_STRING_SPEC_2_HELPER(char, _NAME_, _, _PREFIX_)     \
    STATIC_STRING_SPEC_2_HELPER(wchar_t, _NAME_, _w, _PREFIX_) \
    STATIC_STRING_SPEC_2_HELPER(char8_t, _NAME_, _u8, _PREFIX_)

#if defined(_MSC_VER) && 0 /*currently all versions broken*/
#define STATIC_STRING_SPEC_2_HELPER(_CHAR_T_, _NAME_, _SUFFIX_, _PREFIX_)                                                                      \
    template <size_t Length>                                                                                                                   \
    struct _NAME_##_SUFFIX_##_PREFIX_##string : detail::basic_static_string_full<_CHAR_T_, Length, detail::_NAME_##_##_PREFIX_##string_config> \
    {                                                                                                                                          \
        using detail::basic_static_string_full<_CHAR_T_, Length, detail::_NAME_##_##_PREFIX_##string_config>::basic_static_string_full;        \
    };                                                                                                                                         \
    template <size_t Length>                                                                                                                   \
    _NAME_##_SUFFIX_##_PREFIX_##string(_CHAR_T_ const(&)[Length])->_NAME_##_SUFFIX_##_PREFIX_##string<Length - 1>;

#define STATIC_STRING_SPEC_3_HELPER(_CHAR_T_, _NAME_, _SUFFIX_, _PREFIX_)                                                     \
    template <std::same_as<_CHAR_T_> CharT_override, typename CharT, size_t Length>                                           \
    constexpr _NAME_##_SUFFIX_##_PREFIX_##string<Length - 1> make_##_NAME_##_##_PREFIX_##string(const CharT(&str)[Length])    \
    {                                                                                                                         \
        return {str};                                                                                             \
    }                                                                                                                         \
    template <size_t Length>                                                                                                  \
    constexpr _NAME_##_SUFFIX_##_PREFIX_##string<Length - 1> make_##_NAME_##_##_PREFIX_##string(const _CHAR_T_(&str)[Length]) \
    {                                                                                                                         \
        return {str};                                                                                             \
    }
#define STATIC_STRING_SPEC_3(_NAME_, _PREFIX_)                 \
    STATIC_STRING_SPEC_3_HELPER(char, _NAME_, _, _PREFIX_)     \
    STATIC_STRING_SPEC_3_HELPER(wchar_t, _NAME_, _w, _PREFIX_) \
    STATIC_STRING_SPEC_3_HELPER(char8_t, _NAME_, _u8, _PREFIX_)
#else
#define STATIC_STRING_SPEC_2_HELPER(_CHAR_T_, _NAME_, _SUFFIX_, _PREFIX_) \
    template <size_t Length>                                              \
    using _NAME_##_SUFFIX_##_PREFIX_##string = basic_##_NAME_##_##_PREFIX_##string<_CHAR_T_, Length>;
#define STATIC_STRING_SPEC_3(_NAME_, _PREFIX_)                                                                                              \
    template <typename CharT_override, typename CharT, size_t Length>                                                                       \
    constexpr basic_##_NAME_##_##_PREFIX_##string<CharT_override, Length - 1> make_##_NAME_##_##_PREFIX_##string(const CharT(&str)[Length]) \
        requires(!std::same_as<CharT_override, CharT>)                                                                                      \
    {                                                                                                                                       \
        return {str};                                                                                                           \
    }                                                                                                                                       \
    template <typename CharT, size_t Length>                                                                                                \
    constexpr basic_##_NAME_##_##_PREFIX_##string<CharT, Length - 1> make_##_NAME_##_##_PREFIX_##string(const CharT(&str)[Length])          \
    {                                                                                                                                       \
        return {str};                                                                                                           \
    }
#endif

#define STATIC_STRING_SPEC(_NAME_, _PREFIX_) \
    STATIC_STRING_SPEC_1(_NAME_, _PREFIX_);  \
    STATIC_STRING_SPEC_2(_NAME_, _PREFIX_);  \
    STATIC_STRING_SPEC_3(_NAME_, _PREFIX_);

STATIC_STRING_SPEC(static, );
STATIC_STRING_SPEC(static, c);
STATIC_STRING_SPEC(constant, );
STATIC_STRING_SPEC(constant, c);

#undef STATIC_STRING_SPEC_1
#undef STATIC_STRING_SPEC_2_HELPER
#undef STATIC_STRING_SPEC_2
#undef STATIC_STRING_SPEC_3_HELPER
#undef STATIC_STRING_SPEC_3
#undef STATIC_STRING_SPEC

inline namespace literals
{
#if 0
template <basic_constant_cstring Str>
constexpr auto operator"" _cs() -> typename decltype(Str)::template rebind_config<detail::constant_string_config>
{
    return Str.template set_config<detail::constant_string_config>();
}

template <basic_constant_cstring Str>
constexpr auto operator"" _ss() -> typename decltype(Str)::template rebind_config<detail::static_string_config>
{
    return Str.template set_config<detail::static_string_config>();
}
#else
template <basic_constant_string Str>
constexpr auto operator"" _cs() -> std::remove_const_t<decltype(Str)>
{
    return Str;
}

template <basic_static_string Str>
constexpr auto operator"" _ss() -> std::remove_const_t<decltype(Str)>
{
    return Str;
}
#endif
} // namespace literals
} // namespace fd
