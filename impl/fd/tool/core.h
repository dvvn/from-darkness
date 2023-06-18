#pragma once

#include <type_traits>

#define FD_WRAP_TOOL_SIMPLE(_WRAPPED_, ...) \
    struct _WRAPPED_ : __VA_ARGS__          \
    {                                       \
        using __fd_wrapped = __VA_ARGS__;   \
        using __fd_wrapped::__fd_wrapped;   \
    };

#define FD_WRAP_TOOL_BEGIN(_WRAPPED_, ...)                     \
    struct _WRAPPED_ : __VA_ARGS__                             \
    {                                                          \
        using __fd_wrapped = __VA_ARGS__;                      \
        using __fd_wrapped::__fd_wrapped;                      \
        constexpr _WRAPPED_(__fd_wrapped const &self)          \
            : __fd_wrapped(self)                               \
        {                                                      \
        }                                                      \
        constexpr _WRAPPED_(__fd_wrapped &&self)               \
            : __fd_wrapped(static_cast<__fd_wrapped &&>(self)) \
        {                                                      \
        }

#define FD_WRAP_TOOL_END \
    }                    \
    ;

#if 1
#define FD_WRAP_TOOL(...) FD_WRAP_TOOL_BEGIN(__VA_ARGS__) FD_WRAP_TOOL_END
#else
#define FD_WRAP_TOOL(_WRAPPED_, ...) using _WRAPPED_ = __VA_ARGS__
#endif

namespace fd
{
template <typename T>
concept wrapped = requires { typename T::__fd_wrapped; };

template <wrapped T>
constexpr decltype(auto) unwrap(T &&object)
{
    using raw_t = typename T::__fd_wrapped;
    if constexpr (std::is_rvalue_reference_v<T &&>)
        return raw_t(static_cast<raw_t &&>(object));
    else
    {
        if constexpr (std::is_const_v<T>)
            return static_cast<raw_t const &>(object);
        else
            return static_cast<raw_t &>(object);
    }
}
}