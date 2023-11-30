#pragma once

#include "tier0/core.h"

#define FD_TIER2(_NAME_, ...)  FD_TIER_EX(2, _NAME_, __VA_OPT__(inline) __VA_ARGS__)
#define FD_TIER2i(_NAME_, ...) FD_TIER2(inline _NAME_, __VA_ARGS__)
