#pragma once

#include <fds/math/internal/impl.h>

#include <cmath>

#define FDS_MATH_VEC(_CLASS_NAME_)                                       \
    float _CLASS_NAME_::length() const                                   \
    {                                                                    \
        return std::sqrt(length_sqr());                                  \
    }                                                                    \
    float _CLASS_NAME_::length_sqr() const                               \
    {                                                                    \
        float out      = 0;                                              \
        auto& this_arr = FDS_MATH_AS_ARRAY(_CLASS_NAME_, *this);         \
        for (const auto arg : this_arr)                                  \
            out += arg * arg;                                            \
        return out;                                                      \
    }                                                                    \
    float _CLASS_NAME_::distance_to(const _CLASS_NAME_& other) const     \
    {                                                                    \
        const auto delta = *this - other;                                \
        return delta.length();                                           \
    }                                                                    \
    float _CLASS_NAME_::distance_to_sqr(const _CLASS_NAME_& other) const \
    {                                                                    \
        const auto delta = *this - other;                                \
        return delta.length_sqr();                                       \
    }
