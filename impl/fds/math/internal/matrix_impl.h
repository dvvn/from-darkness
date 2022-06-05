#pragma once

#define FDS_MATH_MATRIX(_CLASS_NAME_)                                                                            \
    _CLASS_NAME_::_CLASS_NAME_(const forward& _forward, const left& _left, const up& _up, const origin& _origin) \
    {                                                                                                            \
        set(_forward);                                                                                           \
        set(_left);                                                                                              \
        set(_up);                                                                                                \
        set(_origin);                                                                                            \
    }                                                                                                            \
    _CLASS_NAME_::_CLASS_NAME_()                                                                                 \
    {                                                                                                            \
    }                                                                                                            \
    void _CLASS_NAME_::set(const forward& _forward)                                                              \
    {                                                                                                            \
        set(0, _forward);                                                                                        \
    }                                                                                                            \
    void _CLASS_NAME_::set(const left& _left)                                                                    \
    {                                                                                                            \
        set(1, _left);                                                                                           \
    }                                                                                                            \
    void _CLASS_NAME_::set(const up& _up)                                                                        \
    {                                                                                                            \
        set(2, _up);                                                                                             \
    }                                                                                                            \
    void _CLASS_NAME_::set(const origin& _origin)                                                                \
    {                                                                                                            \
        set(3, _origin);                                                                                         \
    }                                                                                                            \
    _CLASS_NAME_::operator forward() const                                                                       \
    {                                                                                                            \
        return at(0);                                                                                            \
    }                                                                                                            \
    _CLASS_NAME_::operator left() const                                                                          \
    {                                                                                                            \
        return at(1);                                                                                            \
    }                                                                                                            \
    _CLASS_NAME_::operator up() const                                                                            \
    {                                                                                                            \
        return at(2);                                                                                            \
    }                                                                                                            \
    _CLASS_NAME_::operator origin() const                                                                        \
    {                                                                                                            \
        return at(3);                                                                                            \
    }
