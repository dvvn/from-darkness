#pragma once

#define CHEAT_MATH_VEC_FWD(_CLASS_NAME_) 						\
float length( ) const noexcept;									\
float length_sqr( ) const noexcept;								\
float distance_to(const _CLASS_NAME_& other) const noexcept;	\
float distance_to_sqr(const _CLASS_NAME_& other) const noexcept;\
float normalized() const noexcept;