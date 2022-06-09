#pragma once

#include <cstdint>

#define FDS_BITFLAG_START(_NAME_, ...)                     \
    struct _##_NAME_                                       \
    {                                                      \
        static constexpr size_t _Offset = __COUNTER__ + 1; \
        enum e __VA_OPT__( :) __VA_ARGS__                  \
        {

// clang-format off
#define FDS_BITFLAG_END(_NAME_) \
        };
    };
    using _NAME_ = _##_NAME_::e;
// clang-format on

#define FDS_BITFLAG_ITEM(_NAME_, ...) _NAME_ = 1 << __COUNTER__ - _Offset __VA_OPT__(|) __VA_ARGS__,

// clang-format off
#define PARENS ()

#define EXPAND(...) EXPAND4(EXPAND4(EXPAND4(EXPAND4(__VA_ARGS__))))
#define EXPAND4(...) EXPAND3(EXPAND3(EXPAND3(EXPAND3(__VA_ARGS__))))
#define EXPAND3(...) EXPAND2(EXPAND2(EXPAND2(EXPAND2(__VA_ARGS__))))
#define EXPAND2(...) EXPAND1(EXPAND1(EXPAND1(EXPAND1(__VA_ARGS__))))
#define EXPAND1(...) __VA_ARGS__

#define FOR_EACH(macro, ...)                                    \
  __VA_OPT__(EXPAND(FOR_EACH_HELPER(macro, __VA_ARGS__)))
#define FOR_EACH_HELPER(macro, a1, ...)                         \
  macro(a1)                                                     \
  __VA_OPT__(FOR_EACH_AGAIN PARENS (macro, __VA_ARGS__))
#define FOR_EACH_AGAIN() FOR_EACH_HELPER
// clang-format on

#define FDS_BITFLAG(_NAME_, _TYPE_, ...)    \
    FDS_BITFLAG_START(_NAME_, _TYPE_)       \
    FOR_EACH(FDS_BITFLAG_ITEM, __VA_ARGS__) \
    FDS_BITFLAG_END(_NAME_)
