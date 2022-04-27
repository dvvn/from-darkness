#pragma once

#include <compare>

#define CHEAT_MATH_OP(_RET_,_OP_,_CLASS_NAME_,_CONST_) _RET_ operator##_OP_(const _CLASS_NAME_& other) _CONST_ noexcept;

#ifndef __cpp_lib_three_way_comparison
#define CHEAT_MATH_OP_EQ_FWD(_CLASS_NAME_)  \
CHEAT_MATH_OP(bool,==,_CLASS_NAME_,const)				\
CHEAT_MATH_OP(bool,!=,_CLASS_NAME_,const)				\
CHEAT_MATH_OP(bool,<=,_CLASS_NAME_,const)				\
CHEAT_MATH_OP(bool,<,_CLASS_NAME_,const)				\
CHEAT_MATH_OP(bool,>=,_CLASS_NAME_,const)				\
CHEAT_MATH_OP(bool,>,_CLASS_NAME_,const)
#else
#define CHEAT_MATH_OP_EQ_FWD(_CLASS_NAME_)  \
CHEAT_MATH_OP(bool,==,_CLASS_NAME_,const)				\
CHEAT_MATH_OP(std::partial_ordering,<=>,_CLASS_NAME_,const)
#endif

#define CHEAT_MATH_OP_FWD(_CLASS_NAME_) \
CHEAT_MATH_OP_EQ_FWD(_CLASS_NAME_)		\
CHEAT_MATH_OP(_CLASS_NAME_,+,_CLASS_NAME_,const)	\
CHEAT_MATH_OP(_CLASS_NAME_,-,_CLASS_NAME_,const)	\
CHEAT_MATH_OP(_CLASS_NAME_,*,_CLASS_NAME_,const)	\
CHEAT_MATH_OP(_CLASS_NAME_,/,_CLASS_NAME_,const)	\
CHEAT_MATH_OP(_CLASS_NAME_&,+=,_CLASS_NAME_,)		\
CHEAT_MATH_OP(_CLASS_NAME_&,-=,_CLASS_NAME_,)		\
CHEAT_MATH_OP(_CLASS_NAME_&,*=,_CLASS_NAME_,)		\
CHEAT_MATH_OP(_CLASS_NAME_&,/=,_CLASS_NAME_,)		\

