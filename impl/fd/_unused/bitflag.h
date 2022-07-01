#pragma once

#include <cstdint>

#define FD_BITFLAG_START(_NAME_, ...)                      \
    struct _##_NAME_                                       \
    {                                                      \
        static constexpr size_t _Offset = __COUNTER__ + 1; \
        enum e __VA_OPT__( :) __VA_ARGS__                  \
        {

// clang-format off
#define FD_BITFLAG_END(_NAME_) \
        };
    };
    using _NAME_ = _##_NAME_::e;
// clang-format on

#define FD_BITFLAG_ITEM(_NAME_, ...) _NAME_ = 1 << __COUNTER__ - _Offset __VA_OPT__(|) __VA_ARGS__,

#define FD_BITFLAG(_NAME_, _TYPE_, ...)    \
    FD_BITFLAG_START(_NAME_, _TYPE_)       \
    FOR_EACH(FD_BITFLAG_ITEM, __VA_ARGS__) \
    FD_BITFLAG_END(_NAME_)
