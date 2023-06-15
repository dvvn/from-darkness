#pragma once

#include <concepts>

#define FD_WRAP_TOOL_BEGIN(_WRAPPED_, ...)       \
    struct _WRAPPED_ : __VA_ARGS__               \
    {                                            \
        using _Base = __VA_ARGS__;               \
        using _Base::_Base;                      \
        constexpr _WRAPPED_(_Base const &self)   \
            : _Base(self)                        \
        {                                        \
        }                                        \
        constexpr _WRAPPED_(_Base &&self)        \
            : _Base(static_cast<_Base &&>(self)) \
        {                                        \
        }

#define FD_WRAP_TOOL_END \
    }                    \
    ;

#define FD_WRAP_TOOL(...) FD_WRAP_TOOL_BEGIN(__VA_ARGS__) FD_WRAP_TOOL_END