#pragma once

#include "core.h"

#include <span>

namespace fd
{
template <typename T>
FD_WRAP_TOOL(span, std::span<T>);
}