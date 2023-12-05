#pragma once

#include "container/array.h"
#include "iterator/unwrap.h"

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
class basic_char_table : public array<T, UCHAR_MAX + 1>
{
    using base_array = array<T, UCHAR_MAX + 1>;

  public:
    using pointer = typename base_array::pointer;

  protected:
    using base_array::operator[];
    using base_array::at;

    constexpr std::pair<pointer, pointer> get_range(char const first, char const last)
    {
        auto const abs_first = -CHAR_MIN + first;
        auto const abs_last  = -CHAR_MIN + last + 1;

        auto const first_it = ubegin(*this);

        return {first_it + abs_first, first_it + abs_last};
    }

  public:
    constexpr T operator[](char const c) const
    {
        return base_array::operator[](-CHAR_MIN + c);
    }

    constexpr T& operator[](char const c)
    {
        return base_array::operator[](-CHAR_MIN + c);
    }
};

template <typename T>
struct default_char_table;

template <>
struct default_char_table<char> : basic_char_table<char>
{
    constexpr default_char_table()
    {
#if 0
        auto ptr = this->data();
        for (auto c = CHAR_MIN; c <= CHAR_MAX; ++c)
            *ptr++ = c;
#else
        std::ranges::for_each(*this, [c = CHAR_MIN](char& c_ref) mutable {
            c_ref = c++;
        });
#endif
    }

    template <typename Fn>
    constexpr void transform(char const first, char const last, Fn fn)
    {
        auto const [first_it, last_it] = get_range(first, last);
        std::transform(first_it, last_it, first_it, fn);
    }
};

template <>
struct default_char_table<bool> : basic_char_table<bool>
{
    constexpr default_char_table(bool const value = false)
    {
        this->fill(value);
    }

    constexpr void set(char const first, char const last, bool const value)
    {
        auto const [first_it, last_it] = get_range(first, last);
        std::fill(first_it, last_it, value);
    }
};

struct islower_table : default_char_table<bool>
{
    constexpr islower_table()
    {
        set('a', 'z', true);
    }
};

struct isupper_table : default_char_table<bool>
{
    constexpr isupper_table()
    {
        set('A', 'Z', true);
    }
};

struct isdigit_table : default_char_table<bool>
{
    constexpr isdigit_table()
    {
        set('0', '9', true);
    }
};

struct isxdigit_table : default_char_table<bool>
{
    constexpr isxdigit_table()
    {
        set('0', '9', true);
        set('a', 'f', true);
        set('A', 'F', true);
    }
};

struct tolower_table : default_char_table<char>
{
    constexpr tolower_table()
    {
        transform('A', 'F', [](char const c) {
            return c + ('A' - 'a');
        });
    }
};

struct toupper_table : default_char_table<char>
{
    constexpr toupper_table()
    {
        transform('a', 'f', [](char const c) {
            return c - ('a' - 'A');
        });
    }
};

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
};

inline constexpr char_table_wrapper<islower_table> islower;
inline constexpr char_table_wrapper<isupper_table> isupper;
inline constexpr char_table_wrapper<isdigit_table> isdigit;
inline constexpr char_table_wrapper<isxdigit_table> isxdigit;

inline constexpr char_table_wrapper<toupper_table> toupper;
inline constexpr char_table_wrapper<tolower_table> tolower;
} // namespace fd