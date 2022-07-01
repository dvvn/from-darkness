#pragma once

#include <fd/object.h>

#define _FD_CALLBACK(_CLASS_, _NAME_, _ARG_) FD_OBJECT(_NAME_, _CLASS_<_ARG_>, FD_UNIQUE_INDEX)
#define _FD_CALLBACK_EX(_CLASS_, _NAME_, ...) \
    namespace callbacks                       \
    {                                         \
        using _NAME_ = _CLASS_<__VA_ARGS__>;  \
    }                                         \
    FD_OBJECT(_NAME_, callbacks::_NAME_, FD_UNIQUE_INDEX)
#define _FD_CALLBACK_SELECTOR(_CLASS_, _NAME_, _ARG1_, ...) _FD_CALLBACK##__VA_OPT__(_EX)(_CLASS_, _NAME_, _ARG1_, ##__VA_ARGS__)
