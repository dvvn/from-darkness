#pragma once

#include <type_traits>

namespace fd
{
template <class T, T Value>
using integral_constant = std::integral_constant<T, Value>;

template <bool Value>
using bool_constant = /*std::bool_constant<Value>*/ integral_constant<bool, Value>;
using true_type     = bool_constant<true>;
using false_type    = bool_constant<false>;

template <auto Value>
inline constexpr integral_constant<decltype(Value), Value> integral_constant_v;

template <typename T>
using type_identity = std::type_identity<T>;

template <typename T>
inline constexpr type_identity<T> type_identity_v;

template <bool Test, typename True, typename False>
struct conditional;

template <bool Test, typename True, typename False>
using conditional_t = typename conditional<Test, True, False>::type;

template <typename True, typename False>
struct conditional<true, True, False> : type_identity<True>
{
};

template <typename True, typename False>
struct conditional<false, True, False> : type_identity<False>
{
};

namespace detail
{
template <typename T>
struct next_type_for;

template <>
struct next_type_for<uint8_t> : type_identity<uint16_t>
{
};

template <>
struct next_type_for<uint16_t> : type_identity<uint32_t>
{
};

template <>
struct next_type_for<uint32_t> : type_identity<uint64_t>
{
};

template <>
struct next_type_for<uint64_t> : type_identity<uint64_t>
{
};

template <>
struct next_type_for<int8_t> : type_identity<int16_t>
{
};

template <>
struct next_type_for<int16_t> : type_identity<int32_t>
{
};

template <>
struct next_type_for<int32_t> : type_identity<int64_t>
{
};

template <>
struct next_type_for<int64_t> : type_identity<int64_t>
{
};

template <typename T, T Value>
struct small_type_selector;

template <char Value>
struct small_type_selector<char, Value> : conditional<std::is_signed_v<char>, int8_t, uint8_t>
{
};

template <uint8_t Value>
struct small_type_selector<uint8_t, Value> : type_identity<uint8_t>
{
};

template <uint16_t Value>
struct small_type_selector<uint16_t, Value> : conditional<Value <= UINT8_MAX, uint8_t, uint16_t>
{
};

template <uint32_t Value>
struct small_type_selector<uint32_t, Value> : conditional<Value <= UINT8_MAX, uint8_t, conditional_t<Value <= UINT16_MAX, uint16_t, uint32_t>>
{
};

template <uint64_t Value>
struct small_type_selector<uint64_t, Value> :
    conditional<Value <= UINT8_MAX, uint8_t, conditional<Value <= UINT16_MAX, uint16_t, conditional_t<Value <= UINT32_MAX, uint32_t, uint64_t>>>
{
};

template <int8_t Value>
struct small_type_selector<int8_t, Value> : type_identity<int8_t>
{
};

template <int16_t Value>
struct small_type_selector<int16_t, Value> : conditional<Value <= INT8_MAX, int8_t, int16_t>
{
};

template <int32_t Value>
struct small_type_selector<int32_t, Value> : conditional<Value <= INT8_MAX, int8_t, conditional_t<Value <= INT16_MAX, int16_t, int32_t>>
{
};

template <int64_t Value>
struct small_type_selector<int64_t, Value> :
    conditional<Value <= INT8_MAX, int8_t, conditional<Value <= INT16_MAX, int16_t, conditional_t<Value <= INT32_MAX, int32_t, int64_t>>>
{
};

template <typename T, T V>
using small_type = typename small_type_selector<T, V>::type;

template <class T, T Value>
using small_integral_constant = integral_constant<small_type<T, Value>, Value>;
} // namespace detail

template <auto V>
using small_type = detail::small_type<decltype(V), V>;

template <auto V>
using small_integral_constant = detail::small_integral_constant<decltype(V), V>;

#define _INTEGRAL_CONSTANT_OP(_OP_)                                                               \
    template <typename T_l, T_l Size_l, typename T_r, T_r Size_r>                                 \
    constexpr auto operator##_OP_(integral_constant<T_l, Size_l>, integral_constant<T_r, Size_r>) \
        -> integral_constant<small_type<Size_l _OP_ Size_r>, Size_l _OP_ Size_r>                  \
    {                                                                                             \
        return {};                                                                                \
    }

_INTEGRAL_CONSTANT_OP(+);
_INTEGRAL_CONSTANT_OP(-);
_INTEGRAL_CONSTANT_OP(*);
_INTEGRAL_CONSTANT_OP(/);

template <class T, T Value>
constexpr auto operator-(integral_constant<T, Value>) -> integral_constant<T, -Value>
{
    return {};
}

#undef _INTEGRAL_CONSTANT_OP

namespace detail
{
template <typename Out, char... C>
inline constexpr Out chars_to_num_impl = [] {
    Out num = 0;
    ((num = C - '0' + num * 10), ...);
    return num;
}();

template <typename T, char... C>
using chars_to_num_t = small_type<T, chars_to_num_impl<T, C...>>;

template <char... C>
struct integral_constant_literal : integral_constant<chars_to_num_t<uint64_t, C...>, chars_to_num_impl<uint64_t, C...>>
{
};

template <char C1, char... C>
requires(C1 != '-')
constexpr auto operator-(integral_constant_literal<C1, C...>) -> integral_constant_literal<'-', C1, C...>
{
    return {};
}

template <char... C>
struct integral_constant_literal<'-', C...> : integral_constant<chars_to_num_t<int64_t, C...>, -chars_to_num_impl<int64_t, C...>>
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

template <char... C>
constexpr auto operator""_c() -> detail::integral_constant_literal<C...>
{
    return {};
}

template <typename T, size_t Length>
constexpr auto size(T const (&)[Length]) -> integral_constant<detail::small_type<size_t, Length>, Length>
{
    return {};
}
} // namespace fd