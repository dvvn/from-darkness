﻿#include "global_vars.h"

namespace fd::valve
{
static_assert(sizeof(native_global_vars_base) == 0x40);
static_assert(sizeof(native_global_vars) == 0x64);
} // namespace fd::valve