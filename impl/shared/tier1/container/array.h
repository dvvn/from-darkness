#pragma once
#include "tier0/core.h"

#include <array>

namespace FD_TIER(1)
{
template <typename T, size_t Length>
using array = std::array<T, Length>;
}