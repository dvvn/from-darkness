#pragma once
#include <concepts>
#include <utility>

#ifdef _CONCAT
#define FD_CONCAT _CONCAT
#else
#define FD_CONCATX(x, y) x##y
#define FD_CONCAT(x, y)  FD_CONCATX(x, y)
#endif

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
