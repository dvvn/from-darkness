#pragma once

#include <compare>

#define FD_MATH_OP(_RET_, _OP_, _CLASS_NAME_, _CONST_) _RET_ operator##_OP_(const _CLASS_NAME_& other) _CONST_;

#ifndef __cpp_lib_three_way_comparison
#define FD_MATH_OP_EQ_FWD(_CLASS_NAME_)       \
    FD_MATH_OP(bool, ==, _CLASS_NAME_, const) \
    FD_MATH_OP(bool, !=, _CLASS_NAME_, const) \
    FD_MATH_OP(bool, <=, _CLASS_NAME_, const) \
    FD_MATH_OP(bool, <, _CLASS_NAME_, const)  \
    FD_MATH_OP(bool, >=, _CLASS_NAME_, const) \
    FD_MATH_OP(bool, >, _CLASS_NAME_, const)
#else
#define FD_MATH_OP_EQ_FWD(_CLASS_NAME_)       \
    FD_MATH_OP(bool, ==, _CLASS_NAME_, const) \
    FD_MATH_OP(std::partial_ordering, <=>, _CLASS_NAME_, const)
#endif

#define FD_MATH_OP_FWD(_CLASS_NAME_)                 \
    FD_MATH_OP_EQ_FWD(_CLASS_NAME_)                  \
    FD_MATH_OP(_CLASS_NAME_, +, _CLASS_NAME_, const) \
    FD_MATH_OP(_CLASS_NAME_, -, _CLASS_NAME_, const) \
    FD_MATH_OP(_CLASS_NAME_, *, _CLASS_NAME_, const) \
    FD_MATH_OP(_CLASS_NAME_, /, _CLASS_NAME_, const) \
    FD_MATH_OP(_CLASS_NAME_&, +=, _CLASS_NAME_, )    \
    FD_MATH_OP(_CLASS_NAME_&, -=, _CLASS_NAME_, )    \
    FD_MATH_OP(_CLASS_NAME_&, *=, _CLASS_NAME_, )    \
    FD_MATH_OP(_CLASS_NAME_&, /=, _CLASS_NAME_, )
