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

#define _INTEGRAL_CONSTANT_OP(_OP_)                                                                 \
    template <typename T_l, T_l Value_l, typename T_r, T_r Value_r>                                 \
    constexpr auto operator##_OP_(integral_constant<T_l, Value_l>, integral_constant<T_r, Value_r>) \
        -> integral_constant<decltype(Value_l _OP_ Value_r), Value_l _OP_ Value_r>                  \
    {                                                                                               \
        return {};                                                                                  \
    }

_INTEGRAL_CONSTANT_OP(+);
_INTEGRAL_CONSTANT_OP(-);
_INTEGRAL_CONSTANT_OP(*);
_INTEGRAL_CONSTANT_OP(/);

#undef _INTEGRAL_CONSTANT_OP

#define _INTEGRAL_CONSTANT_OP_SELF(_OP_)                                                                                \
    template <typename T, T Value>                                                                                      \
    constexpr auto operator##_OP_(integral_constant<T, Value>) -> integral_constant<decltype(_OP_##Value), _OP_##Value> \
    {                                                                                                                   \
        return {};                                                                                                      \
    }

_INTEGRAL_CONSTANT_OP_SELF(-);
_INTEGRAL_CONSTANT_OP_SELF(~);
_INTEGRAL_CONSTANT_OP_SELF(!);

#undef _INTEGRAL_CONSTANT_OP_SELF

namespace detail
{
template <typename Out, char... C>
struct chars_literal_to_num;
}
} // namespace fd