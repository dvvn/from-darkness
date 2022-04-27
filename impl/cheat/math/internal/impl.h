#pragma once

#include <nstd/core.h>

#include <array>

#define STD_ARRAY_OP(_OP_)   											  								 \
template<typename T, size_t S>											  								 \
auto& NSTD_CONCAT(operator,_OP_##=)(std::array<T, S>& l, const std::array<T, S>& r) noexcept			 \
{																		  								 \
	for(size_t i = 0; i < S; ++i)									  									 \
		l[i] _OP_##= r[i];													  							 \
	return l;																							 \
}																										 \
template<typename T, size_t S>																			 \
auto NSTD_CONCAT(operator,_OP_)(std::array<T, S> l, const std::array<T, S>& r) noexcept	  				 \
{																		  								 \
	l _OP_##= r;														  								 \
	return l;																							 \
}

namespace std
{
	STD_ARRAY_OP(+);
	STD_ARRAY_OP(-);
	STD_ARRAY_OP(*);
	STD_ARRAY_OP(/);
}

template<class ArrT, class T>
auto& _As_array(T& obj) noexcept
{
	using arr_t = std::conditional_t<std::is_const_v<T>, const ArrT, ArrT>;
	return reinterpret_cast<arr_t&>(obj);
}

#define CHEAT_MATH_ARRAY_INIT(_CLASS_NAME_,_ARG_TYPE_,_ARGS_COUNT_) 										   \
static_assert(sizeof(_CLASS_NAME_) == sizeof(_ARG_TYPE_) * _ARGS_COUNT_, #_CLASS_NAME_": alignment detected"); \
using _CLASS_NAME_##_arr = std::array<_ARG_TYPE_,_ARGS_COUNT_>;

#define CHEAT_MATH_AS_ARRAY(_CLASS_NAME_,_OBJ_) _As_array<_CLASS_NAME_##_arr>(_OBJ_)

#define CHEAT_MATH_OP_EQ_IMPL(_CLASS_NAME_,_RET_,_OP_)									 \
_RET_ _CLASS_NAME_::NSTD_CONCAT(operator,_OP_)(const _CLASS_NAME_& other) const noexcept \
{																					     \
	auto &buff_this = CHEAT_MATH_AS_ARRAY(_CLASS_NAME_, *this);							 \
	auto &buff_other = CHEAT_MATH_AS_ARRAY(_CLASS_NAME_, other);						 \
	return buff_this _OP_ buff_other;													 \
}

#ifndef __cpp_lib_three_way_comparison
#define CHEAT_MATH_OP_EQ(_CLASS_NAME_)		  \
CHEAT_MATH_OP_EQ_IMPL(_CLASS_NAME_,bool,==);  \
CHEAT_MATH_OP_EQ_IMPL(_CLASS_NAME_,bool,!=);  \
CHEAT_MATH_OP_EQ_IMPL(_CLASS_NAME_,bool,<);	  \
CHEAT_MATH_OP_EQ_IMPL(_CLASS_NAME_,bool,<=);  \
CHEAT_MATH_OP_EQ_IMPL(_CLASS_NAME_,bool,>);	  \
CHEAT_MATH_OP_EQ_IMPL(_CLASS_NAME_,bool,>=);
#else
#define CHEAT_MATH_OP_EQ(_CLASS_NAME_)						   \
CHEAT_MATH_OP_EQ_IMPL(_CLASS_NAME_,bool,==);				   \
CHEAT_MATH_OP_EQ_IMPL(_CLASS_NAME_,std::partial_ordering,<=>); 
#endif

#define CHEAT_MATH_OP_IMPL(_CLASS_NAME_,_OP_) 															 \
_CLASS_NAME_ _CLASS_NAME_::NSTD_CONCAT(operator,_OP_)(const _CLASS_NAME_& other) const noexcept			 \
{																										 \
	auto &buff_this = CHEAT_MATH_AS_ARRAY(_CLASS_NAME_, *this);											 \
	auto &buff_other = CHEAT_MATH_AS_ARRAY(_CLASS_NAME_, other);										 \
	auto tmp = buff_this _OP_ buff_other;																 \
	return reinterpret_cast<_CLASS_NAME_&&>(tmp);														 \
}																										 \
_CLASS_NAME_& _CLASS_NAME_::NSTD_CONCAT(operator,_OP_##=)(const _CLASS_NAME_& other) noexcept			 \
{																										 \
	auto &buff_this = CHEAT_MATH_AS_ARRAY(_CLASS_NAME_, *this);											 \
	auto &buff_other = CHEAT_MATH_AS_ARRAY(_CLASS_NAME_, other);										 \
	buff_this _OP_##= buff_other;																		 \
	return *this;																						 \
}

#define CHEAT_MATH_OP(_CLASS_NAME_)	\
CHEAT_MATH_OP_IMPL(_CLASS_NAME_,+)	\
CHEAT_MATH_OP_IMPL(_CLASS_NAME_,-)	\
CHEAT_MATH_OP_IMPL(_CLASS_NAME_,*)	\
CHEAT_MATH_OP_IMPL(_CLASS_NAME_,/)	\
CHEAT_MATH_OP_EQ(_CLASS_NAME_)