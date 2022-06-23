#pragma once

#include <fd/core/callback/internal.h>

import fd.callback;

#define FD_CALLBACK(_NAME_, ...)                                                       \
    static_assert(_Is_header_file(__FILE__), "Interface made specially for headers!"); \
    FD_CALLBACK_SELECTOR(fd::abstract_callback, _NAME_, __VA_ARGS__)
