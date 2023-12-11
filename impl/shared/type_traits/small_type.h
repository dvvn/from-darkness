#pragma once

#include "type_traits/conditional.h"

namespace fd
{
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
} // namespace detail

// template <auto V>
// using small_type = typename detail::small_type_selector<decltype(V), V>::type;

template <typename T, T V>
using small_type = typename detail::small_type_selector<T, V>::type;
} // namespace fd