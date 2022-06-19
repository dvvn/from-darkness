#pragma once

#include <fd/core/object.h>

import fd.callback;

#define FD_CALLBACK(_NAME_, ...)                           \
    namespace callbacks                                     \
    {                                                       \
        using _NAME_ = fd::abstract_callback<__VA_ARGS__>; \
    }                                                       \
    FD_UNIQUE_OBJECT(_NAME_, callbacks::_NAME_)
