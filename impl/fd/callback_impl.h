#pragma once

#include <fd/callback/internal.h>

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

#define FD_CALLBACK_BIND(_NAME_, ...)                                                                    \
    FD_OBJECT_BIND_TYPE(_NAME_,                                                                     /**/ \
                        _FIRST_ARG(__VA_ARGS__ __VA_OPT__(, ) decltype(_Rewrap_callback(*_NAME_))), /**/ \
                        _FIRST_ARG(__VA_OPT__(0, ) _NAME_))

#define FD_CALLBACK(_NAME_, ...) FD_CALLBACK_SELECTOR(fd::callback, _NAME_, __VA_ARGS__)
