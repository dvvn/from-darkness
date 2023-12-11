#pragma once

#include "type_traits/integral_constant.h"
#include "type_traits/small_type.h"

namespace fd
{
namespace detail
{
template <typename Out, char... C>
struct chars_literal_to_num
{
    static constexpr Out value = [] {
        Out num = 0;
        ((num = C - '0' + num * 10), ...);
        return num;
    }();
};

template <typename Out, char... C>
requires(!std::same_as<Out, uint64_t>)
struct chars_literal_to_num<Out, C...>
{
    static constexpr Out value = static_cast<Out>(chars_literal_to_num<uint64_t, C...>::value);
};

template <typename T, char... C>
using chars_to_num_t = small_type<T, chars_literal_to_num<T, C...>::value>;

template <char... C>
struct integral_constant_literal : integral_constant<chars_to_num_t<uint64_t, C...>, chars_literal_to_num<uint64_t, C...>::value>
{
};

template <char C1, char... C>
requires(C1 != '-')
constexpr auto operator-(integral_constant_literal<C1, C...>) -> integral_constant_literal<'-', C1, C...>
{
    return {};
}

template <char... C>
struct integral_constant_literal<'-', C...> : integral_constant<chars_to_num_t<int64_t, C...>, -chars_literal_to_num<int64_t, C...>::value>
{
};

#ifdef _DEBUG
#define _INTEGRAL_CONSTANT_LITERAL_PRESET(_N_)                                       \
    template <>                                                                      \
    struct integral_constant_literal<'-', #_N_[0]> : integral_constant<int8_t, -_N_> \
    {                                                                                \
    };                                                                               \
    template <>                                                                      \
    struct integral_constant_literal<#_N_[0]> : integral_constant<uint8_t, _N_>      \
    {                                                                                \
    };

_INTEGRAL_CONSTANT_LITERAL_PRESET(0);
_INTEGRAL_CONSTANT_LITERAL_PRESET(1);
_INTEGRAL_CONSTANT_LITERAL_PRESET(2);
_INTEGRAL_CONSTANT_LITERAL_PRESET(3);
_INTEGRAL_CONSTANT_LITERAL_PRESET(4);
_INTEGRAL_CONSTANT_LITERAL_PRESET(5);
_INTEGRAL_CONSTANT_LITERAL_PRESET(6);
_INTEGRAL_CONSTANT_LITERAL_PRESET(7);
_INTEGRAL_CONSTANT_LITERAL_PRESET(8);
_INTEGRAL_CONSTANT_LITERAL_PRESET(9);

#undef _INTEGRAL_CONSTANT_LITERAL_PRESET
#endif

} // namespace detail

inline namespace literals
{
template <char... C>
constexpr auto operator""_c() -> detail::integral_constant_literal<C...>
{
    return {};
}
} // namespace literals
} // namespace fd