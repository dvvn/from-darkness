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

/* #ifdef _CONCAT
#define _FD_CONCAT _CONCAT
#else
#define _FD_CONCATX(x, y) x##y
#define _FD_CONCAT(x, y)  _FD_CONCATX(x, y)
#endif */
#define _FD_UNPACK(x) x

#define FD_CONCAT(...) FOR_EACH(_FD_UNPACK, __VA_ARGS__)

#ifdef _STRINGIZE
#define FD_STRINGIZE _STRINGIZE
#else
#define FD_STRINGIZEX(x) #x
#define FD_STRINGIZE(x)  FD_STRINGIZEX(x)
#endif

/* #ifdef _CRT_WIDE
#define FD_STRINGIZE_WIDE _CRT_WIDE
#else
#define FD_STRINGIZE_WIDE(x) FD_CONCAT(L, FD_STRINGIZE(x))
#endif

#define FD_STRINGIZE_RAW(x)      FD_CONCAT(R, FD_STRINGIZE(##(x)##))
#define FD_STRINGIZE_RAW_WIDE(x) FD_CONCAT(L, FD_STRINGIZE_RAW(x)) */

namespace fd
{
    /* template <typename T>
    concept has_array_access = requires(const T & obj)
    {
        obj[0u];
    };

    template <class _Ty>
    concept _Has_member_allocator_type = requires {
        typename _Ty::allocator_type;
    };

    template <class T, typename New>
    using rebind_helper = typename std::_Replace_first_parameter<New, T>::type; */

    template <typename T>
    struct remove_all_pointers : std::conditional_t<std::is_pointer_v<T>, remove_all_pointers<std::remove_pointer_t<T>>, std::type_identity<T>>
    {
    };

    template <typename T>
    using remove_all_pointers_t = typename remove_all_pointers<T>::type;

    template <bool Val, typename T>
    using add_const_if_v = std::conditional_t<Val, std::add_const_t<T>, T>;

    template <typename Test, typename T>
    using add_const_if = add_const_if_v<std::is_const_v<Test>, T>;

#ifdef __cpp_lib_unreachable
    using std::unreachable;
#else
    [[noreturn]] inline void unreachable()
    {
        std::abort();
    }
#endif
} // namespace fd
