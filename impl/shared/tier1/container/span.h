#pragma once
#include "tier0/core.h"

#include <span>

namespace FD_TIER(1)
{
template <typename T>
using span = std::span<T>;
}