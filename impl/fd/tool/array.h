#pragma once

#include "core.h"

#include <array>

namespace fd
{
template <typename T, size_t Length>
FD_WRAP_TOOL_SIMPLE(array, std::array<T, Length>);
}