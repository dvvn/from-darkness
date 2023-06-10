#pragma once

#define FD_WRAP_TOOL(_WRAPPED_, ...)      \
    struct _WRAPPED_ : __VA_ARGS__        \
    {                                     \
        using _Base = __VA_ARGS__;        \
        using _Base::_Base;               \
        constexpr _WRAPPED_(_Base &&self) \
            : _Base(move(self))           \
        {                                 \
        }                                 \
    };