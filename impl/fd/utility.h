#pragma once
#include <concepts>
#include <utility>

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

#ifdef _CONCAT
#define FD_CONCAT _CONCAT
#else
#define _FD_CONCATX(x, y) x##y
#define FD_CONCAT(x, y)   _FD_CONCATX(x, y)
#endif
#define _FD_UNPACK(x) x

#define FD_CONCAT_EX(...) FOR_EACH(_FD_UNPACK, __VA_ARGS__)

#if 0
#define FD_STRINGIZE(x) #x
#else
#ifdef _STRINGIZE
#define FD_STRINGIZE _STRINGIZE
#else
#define FD_STRINGIZEX(x) #x
#define FD_STRINGIZE(x)  FD_STRINGIZEX(x)
#endif
#endif

/* #ifdef _CRT_WIDE
#define FD_STRINGIZE_WIDE _CRT_WIDE
#else
#define FD_STRINGIZE_WIDE(x) FD_CONCAT(L, FD_STRINGIZE(x))
#endif

#define FD_STRINGIZE_RAW(x)      FD_CONCAT(R, FD_STRINGIZE(##(x)##))
#define FD_STRINGIZE_RAW_WIDE(x) FD_CONCAT(L, FD_STRINGIZE_RAW(x)) */
