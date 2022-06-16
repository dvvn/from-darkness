#pragma once

#include <fds/core/object.h>

import fds.callback.impl;

template <typename... Args>
fds::callback<Args...> _Rewrap_callback(const fds::abstract_callback<Args...>&)
{
    return {};
}

template <typename... Args>
fds::callback<Args...> _Rewrap_callback(const fds::callback<Args...>&) = delete;

template <class T>
using _Rewrap_callback_t = decltype(_Rewrap_callback(std::declval<T>()));

#define _FIRST_ARG(a, ...) a

#define FDS_CALLBACK_BIND(_NAME_, ...)                                                                 \
    FDS_OBJECT_BIND(callbacks::_NAME_, _NAME_,                                                    /**/ \
                    _FIRST_ARG(__VA_ARGS__ __VA_OPT__(, ) _Rewrap_callback_t<callbacks::_NAME_>), /**/ \
                    _FIRST_ARG(__VA_OPT__(0, ) _NAME_))

#define FDS_CALLBACK(_NAME_, ...)                  \
    namespace callbacks                            \
    {                                              \
        using _NAME_ = fds::callback<__VA_ARGS__>; \
    }                                              \
    FDS_UNIQUE_OBJECT(_NAME_, callbacks::_NAME_)
