#pragma once

#include "container/array.h"

#include <algorithm>
#include <cassert>
#include <concepts>

namespace fd
{
template <size_t S>
[[deprecated]]
constexpr size_t strlen(char const (&str)[S]) noexcept
{
    assert(*std::rbegin(str) == '\0');
    return S - 1;
}

template <typename T>
struct basic_char_table : array<T, UCHAR_MAX + 1>
{
  protected:
    using base_array = array<T, UCHAR_MAX + 1>;

    using base_array::operator[];
    using base_array::at;

  public:
    using pointer   = typename base_array::pointer;
    using size_type = typename base_array::size_type;

  private:
    static constexpr size_type get_index(char const chr)
    {
#if 1
        return -CHAR_MIN + chr;
#else
        return static_cast<unsigned char>(chr);
#endif
    }

    constexpr std::pair<pointer, pointer> get_range(char const first, char const last)
    {
        auto const abs_first = get_index(first);
        auto const abs_last  = get_index(last) + 1;

        assert(abs_first < abs_last);

        auto const first_it = base_array::data();
        return {first_it + abs_first, first_it + abs_last};
    }

  public:
    /*constexpr basic_char_table()
        : base_array{}
    {
    }*/

    constexpr T operator[](char const c) const
    {
        return base_array::operator[](get_index(c));
    }

    constexpr T& operator[](char const c)
    {
        return base_array::operator[](get_index(c));
    }

    //---

    constexpr void set(char const index, T const value)
    {
        operator[](index) = value;
    }

    constexpr void set(char const first, char const last, T const value)
    {
        auto const [first_it, last_it] = get_range(first, last);
        std::fill(first_it, last_it, value);
    }

    template <typename Fn>
    constexpr void transform(char const first, char const last, Fn fn)
    {
        auto const [first_it, last_it] = get_range(first, last);
        std::transform(first_it, last_it, first_it, fn);
    }
};

namespace detail
{
template <typename T>
inline constexpr void* default_char_table = nullptr;

template <>
inline constexpr auto default_char_table<char> = [] {
    basic_char_table<char> ret;

    auto const data       = ret.data();
    auto const last_index = ret.size();
    for (uint16_t index = 0; index != last_index; ++index)
        data[index] = CHAR_MIN + index;

    return ret;
}();

template <>
inline constexpr auto default_char_table<bool> = [] {
    basic_char_table<bool> ret;
    ret.fill(false);
    return ret;
}();

struct islower_table final : basic_char_table<bool>
{
    constexpr islower_table()
        : basic_char_table{default_char_table<bool>}
    {
        set('a', 'z', true);
    }
};

struct isupper_table final : basic_char_table<bool>
{
    constexpr isupper_table()
        : basic_char_table{default_char_table<bool>}
    {
        set('A', 'Z', true);
    }
};

struct isdigit_table final : basic_char_table<bool>
{
    constexpr isdigit_table()
        : basic_char_table{default_char_table<bool>}
    {
        set('0', '9', true);
    }
};

struct isxdigit_table final : basic_char_table<bool>
{
    constexpr isxdigit_table()
        : basic_char_table{default_char_table<bool>}
    {
        set('0', '9', true);
        set('a', 'f', true);
        set('A', 'F', true);
    }
};

struct tolower_table final : basic_char_table<char>
{
    constexpr tolower_table()
        : basic_char_table{default_char_table<char>}
    {
        transform('A', 'F', [](char const c) {
            return c + ('A' - 'a');
        });
    }
};

struct toupper_table final : basic_char_table<char>
{
    constexpr toupper_table()
        : basic_char_table{default_char_table<char>}
    {
        transform('a', 'f', [](char const c) {
            return c - ('a' - 'A');
        });
    }
};
} // namespace detail

template <class T>
class char_table_wrapper
{
    T table_;

  public:
    template <std::same_as<char> C>
    constexpr typename T::value_type operator()(C const val) const
    {
        return table_[val];
    }

    constexpr T const& table() const
    {
        return table_;
    }
};

inline constexpr char_table_wrapper<detail::islower_table> islower;
inline constexpr char_table_wrapper<detail::isupper_table> isupper;
inline constexpr char_table_wrapper<detail::isdigit_table> isdigit;
inline constexpr char_table_wrapper<detail::isxdigit_table> isxdigit;

inline constexpr char_table_wrapper<detail::toupper_table> toupper;
inline constexpr char_table_wrapper<detail::tolower_table> tolower;
} // namespace fd