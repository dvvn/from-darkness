#pragma once

#include "internal.h"

import fd.callback.impl;

template <typename... Args>
fd::callback<Args...> _Rewrap_callback(const fd::abstract_callback<Args...>&)
{
    return {};
}

template <class C>
fd::callback_custom<C> _Rewrap_callback(const fd::abstract_callback_custom<C>&)
{
    return {};
}

#define FD_CALLBACK_TYPE(_NAME_) decltype(_Rewrap_callback(*_NAME_))

#define _FD_CALLBACK_BIND(_NAME_)                     FD_OBJECT_BIND_TYPE(_NAME_, FD_CALLBACK_TYPE(_NAME_))
#define _FD_CALLBACK_BIND_CUSTOM(_NAME_, _OVERRIDEN_) FD_OBJECT_BIND_TYPE(_NAME_, _OVERRIDEN_)
#define FD_CALLBACK_BIND(_NAME_, ...)                 _FD_CALLBACK_BIND##__VA_OPT__(_CUSTOM)(_NAME_, ##__VA_ARGS__)

#define FD_CALLBACK(_NAME_, ...) _FD_CALLBACK_SELECTOR(fd::callback, _NAME_, __VA_ARGS__)
