#pragma once

#include <fds/core/event_fwd.h>

import fds.event;

#define FDS_EVENT(_NAME_, ...)                  \
    namespace events                            \
    {                                           \
        using _NAME_ = fds::event<__VA_ARGS__>; \
    }                                           \
    FDS_UNIQUE_OBJECT(_NAME_, events::_NAME_)

template <typename... Args>
fds::event<Args...> _Rewrap_event(const fds::abstract_event<Args...>&)
{
    return {};
}

#define _FIRST_ARG(a, ...) a

#define FDS_EVENT_BIND(_NAME_, ...)                                                                                      \
    FDS_OBJECT_BIND(events::_NAME_, _NAME_,                                                                         /**/ \
                    _FIRST_ARG(__VA_ARGS__ __VA_OPT__(, ) decltype(_Rewrap_event(std::declval<events::_NAME_>()))), /**/ \
                    _FIRST_ARG(__VA_OPT__(0, ) _NAME_))
