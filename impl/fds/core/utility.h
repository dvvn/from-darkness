#pragma once
#include <concepts>
#include <utility>

#ifdef _CONCAT
#define FDS_CONCAT _CONCAT
#else
#define FDS_CONCATX(x, y) x##y
#define FDS_CONCAT(x, y)  FDS_CONCATX(x, y)
#endif

#ifdef _STRINGIZE
#define FDS_STRINGIZE _STRINGIZE
#else
#define FDS_STRINGIZEX(x) #x
#define FDS_STRINGIZE(x)  FDS_STRINGIZEX(x)
#endif

#ifdef _CRT_WIDE
#define FDS_STRINGIZE_WIDE _CRT_WIDE
#else
#define FDS_STRINGIZE_WIDE(x) FDS_CONCAT(L, FDS_STRINGIZE(x))
#endif

#define FDS_STRINGIZE_RAW(x)      FDS_CONCAT(R, FDS_STRINGIZE(##(x)##))
#define FDS_STRINGIZE_RAW_WIDE(x) FDS_CONCAT(L, FDS_STRINGIZE_RAW(x))

namespace fds
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
} // namespace fds
