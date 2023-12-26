#pragma once

#include "container/array.h"

#include <algorithm>
#include <cassert>
#include <concepts>

namespace fd
{
template <typename T, size_t Size = CHAR_MAX>
struct basic_char_table : array<T, Size + 1>
{
  protected:
    using base_array = array<T, Size + 1>;

    using base_array::operator[];
    using base_array::at;

  public:
    using pointer   = typename base_array::pointer;
    using size_type = typename base_array::size_type;

  private:
    constexpr std::pair<pointer, size_type> get_range(char const front, char const back)
    {
        assert(front < back);
        assert(front >= 0);
        assert(back <= Size);
        return {base_array::data() + front, static_cast<size_type>(back) - static_cast<size_type>(front) + 1};
    }

  public:
    basic_char_table() = default;

    template <typename T1, size_t Size1>
    constexpr basic_char_table(basic_char_table<T1, Size1> const& other)
#ifdef _DEBUG
        requires(Size <= Size1)
#endif
    {
        std::copy_n(other.data(), this->size(), this->data());
    }

    constexpr T operator[](char const c) const
    {
        return base_array::operator[](c);
    }

    constexpr T& operator[](char const c)
    {
        return base_array::operator[](c);
    }

    //---

    constexpr void set(char const index, T const value)
    {
        operator[](index) = value;
    }

    constexpr void set(char const front, char const back, T const value)
    {
        auto const [it, count] = get_range(front, back);
        std::fill_n(it, count, value);
    }

    constexpr void set(char const front, char const back, char const new_front, char const new_back)
    {
        auto [dst_it, count]  = get_range(front, back);
        auto [src_it, count2] = get_range(new_front, new_back);
        assert(count == count2);
        std::copy_n(src_it, count, dst_it);
    }
};

template <typename T, size_t Size>
requires(Size > CHAR_MAX)
struct basic_char_table<T, Size>;

namespace detail
{
template <typename T, size_t Size>
constexpr void init_default_char_table(basic_char_table<T, Size>& table)
{
    auto const last_index = table.size();
    for (size_t index = 0; index != last_index; ++index)
        table[static_cast<char>(index)] = static_cast<T>(index);
}

template <size_t Size>
constexpr void init_default_char_table(basic_char_table<bool, Size>& table)
{
    table.fill(false);
}

template <typename T, size_t Size = CHAR_MAX>
struct default_char_table : basic_char_table<T, Size>
{
    constexpr default_char_table()
    {
        init_default_char_table(*this);
    }

    template <typename T1, size_t Size1>
    constexpr default_char_table(basic_char_table<T1, Size1> const& sample)
#ifdef _DEBUG
        requires(Size <= Size1)
#endif
    {
        std::copy_n(sample.data(), this->size(), this->data());
    }
};

template <typename T>
inline constexpr default_char_table<T> default_char_table_v;

struct islower_table final : basic_char_table<bool>
{
    constexpr islower_table()
        : basic_char_table{default_char_table_v<bool>}
    {
        set('a', 'z', true);
    }
};

struct isupper_table final : basic_char_table<bool>
{
    constexpr isupper_table()
        : basic_char_table{default_char_table_v<bool>}
    {
        set('A', 'Z', true);
    }
};

struct isdigit_table final : basic_char_table<bool>
{
    constexpr isdigit_table()
        : basic_char_table{default_char_table_v<bool>}
    {
        set('0', '9', true);
    }
};

struct isxdigit_table final : basic_char_table<bool>
{
    constexpr isxdigit_table()
        : basic_char_table{default_char_table_v<bool>}
    {
        set('0', '9', true);
        set('a', 'f', true);
        set('A', 'F', true);
    }
};

struct tolower_table final : basic_char_table<char>
{
    constexpr tolower_table()
        : basic_char_table{default_char_table_v<char>}
    {
        set('A', 'Z', 'a', 'z');
    }
};

struct toupper_table final : basic_char_table<char>
{
    constexpr toupper_table()
        : basic_char_table{default_char_table_v<char>}
    {
        set('a', 'z', 'A', 'Z');
    }
};
} // namespace detail

template <class T>
class char_table_wrapper final
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

inline constexpr char_table_wrapper<detail::tolower_table> tolower;
inline constexpr char_table_wrapper<detail::toupper_table> toupper;

namespace detail
{
struct char_to_num_table : basic_char_table<uint8_t, '9' + 1>
{
    constexpr char_to_num_table()
        : basic_char_table{default_char_table_v<uint8_t>}
    {
        set('0', '9', 0, 9);
    }
};

inline constexpr char_table_wrapper<char_to_num_table> char_to_num;

template <char... C>
struct chars_literal_to_num<uint64_t, C...>
{
    static constexpr uint64_t value = [] {
        uint64_t num = 0;
        ((num = char_to_num(C) + num * 10), ...);
        return num;
    }();
};

} // namespace detail
} // namespace fd