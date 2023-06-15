#pragma once

#include "core.h"

#include <variant>

namespace fd
{
template <typename... Types>
FD_WRAP_TOOL(variant, std::variant<Types...>);
using std::visit;
}