#pragma once

#define MATRIX_DIR(_NAME_, _BASE_) \
    struct _NAME_ : _BASE_         \
    {                              \
        using _BASE_::_BASE_;      \
        _NAME_(const _BASE_& base) \
            : _BASE_(base)         \
        {                          \
        }                          \
    };

#define FD_MATH_MATRIX_FWD(_CLASS_NAME_, _VIEW_)                                                    \
    MATRIX_DIR(forward, _VIEW_);                                                                    \
    MATRIX_DIR(left, _VIEW_);                                                                       \
    MATRIX_DIR(up, _VIEW_);                                                                         \
    MATRIX_DIR(origin, _VIEW_);                                                                     \
    _CLASS_NAME_(const forward& _forward, const left& _left, const up& _up, const origin& _origin); \
    _CLASS_NAME_();                                                                                 \
    void set(const size_t index, const _VIEW_& vec);                                                \
    _VIEW_ at(const size_t index) const;                                                            \
    void set(const forward& _forward);                                                              \
    void set(const left& _left);                                                                    \
    void set(const up& _up);                                                                        \
    void set(const origin& _origin);                                                                \
    operator forward() const;                                                                       \
    operator left() const;                                                                          \
    operator up() const;                                                                            \
    operator origin() const;
