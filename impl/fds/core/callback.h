#pragma once

#include <fds/core/object.h>

import fds.callback;

#define FDS_CALLBACK(_NAME_, ...)                           \
    namespace callbacks                                     \
    {                                                       \
        using _NAME_ = fds::abstract_callback<__VA_ARGS__>; \
    }                                                       \
    FDS_UNIQUE_OBJECT(_NAME_, callbacks::_NAME_)
