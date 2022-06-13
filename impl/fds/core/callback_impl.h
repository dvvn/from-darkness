#pragma once

#include <fds/core/object.h>

import fds.callback.impl;

template <typename... Args>
fds::callback<Args...> _Rewrap_callback(const fds::abstract_callback<Args...>&)
{
    return {};
}

#define _FIRST_ARG(a, ...) a

#define FDS_CALLBACK_BIND(_NAME_, ...)                                                                                         \
    FDS_OBJECT_BIND(callbacks::_NAME_, _NAME_,                                                                            /**/ \
                    _FIRST_ARG(__VA_ARGS__ __VA_OPT__(, ) decltype(_Rewrap_callback(std::declval<callbacks::_NAME_>()))), /**/ \
                    _FIRST_ARG(__VA_OPT__(0, ) _NAME_))
