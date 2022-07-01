#pragma once

#include <fd/callback_internal.h>

import fd.callback;

#define FD_CALLBACK(_NAME_, ...) _FD_CALLBACK_SELECTOR(fd::abstract_callback, _NAME_, __VA_ARGS__)
