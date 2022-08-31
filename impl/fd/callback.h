#pragma once

#include <fd/callback/internal.h>

import fd.callback;

#define FD_CALLBACK(_NAME_, ...) _FD_CALLBACK_SELECTOR(fd::abstract_callback, _NAME_, __VA_ARGS__)
#define FD_CALLBACK_CUSTOM(_NAME_, _TYPE_) _FD_CALLBACK_SELECTOR(fd::abstract_callback_custom, _NAME_, _TYPE_)
