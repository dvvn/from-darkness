#pragma once

#include <fd/core/object.h>

import fd.callback.impl;

template <typename... Args>
fd::callback<Args...> _Rewrap_callback(const fd::abstract_callback<Args...>&)
{
    return {};
}

template <typename... Args>
fd::callback<Args...> _Rewrap_callback(const fd::callback<Args...>&) = delete;

template <class T>
using _Rewrap_callback_t = decltype(_Rewrap_callback(std::declval<T>()));

#define _FIRST_ARG(a, ...) a

#define FD_CALLBACK_BIND(_NAME_, ...)                                                                 \
    FD_OBJECT_BIND(callbacks::_NAME_, _NAME_,                                                    /**/ \
                   _FIRST_ARG(__VA_ARGS__ __VA_OPT__(, ) _Rewrap_callback_t<callbacks::_NAME_>), /**/ \
                   _FIRST_ARG(__VA_OPT__(0, ) _NAME_))

#define FD_CALLBACK(_NAME_, ...)                  \
    namespace callbacks                           \
    {                                             \
        using _NAME_ = fd::callback<__VA_ARGS__>; \
    }                                             \
    FD_UNIQUE_OBJECT(_NAME_, callbacks::_NAME_)
