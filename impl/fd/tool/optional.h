#pragma once

#include "core.h"

#include <optional>

namespace fd
{
template <typename T>
FD_WRAP_TOOL(optional, std::optional<T>);
}