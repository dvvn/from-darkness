#pragma once

#include <fds/core/object.h>

import fds.event.abstract;

#define FDS_EVENT_FWD(_NAME_, ...)                       \
    namespace events                                     \
    {                                                    \
        using _NAME_ = fds::abstract_event<__VA_ARGS__>; \
    }                                                    \
    FDS_UNIQUE_OBJECT(_NAME_, events::_NAME_)
