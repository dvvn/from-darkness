#pragma once

#define FD_WRAP_TOOL_BEGIN_SIMPLE(_WRAPPED_, ...) \
    struct _WRAPPED_ : __VA_ARGS__                \
    {                                             \
        using __fd_wrapped = __VA_ARGS__;         \
        using __fd_wrapped::__fd_wrapped;

#define FD_WRAP_TOOL_BEGIN(_WRAPPED_, ...)                 \
    FD_WRAP_TOOL_BEGIN_SIMPLE(_WRAPPED_, __VA_ARGS__)      \
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

#define FD_WRAP_TOOL_SIMPLE(_WRAPPED_, ...)           \
    FD_WRAP_TOOL_BEGIN_SIMPLE(_WRAPPED_, __VA_ARGS__) \
    FD_WRAP_TOOL_END

#if 1
#define FD_WRAP_TOOL(...) FD_WRAP_TOOL_BEGIN(__VA_ARGS__) FD_WRAP_TOOL_END
#else
#define FD_WRAP_TOOL(_WRAPPED_, ...) using _WRAPPED_ = __VA_ARGS__
#endif