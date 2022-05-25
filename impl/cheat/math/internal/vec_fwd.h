#pragma once

#define CHEAT_MATH_VEC_FWD(_CLASS_NAME_)                    \
    float length() const;                                   \
    float length_sqr() const;                               \
    float distance_to(const _CLASS_NAME_& other) const;     \
    float distance_to_sqr(const _CLASS_NAME_& other) const; \
    float normalized() const;
