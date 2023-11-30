#pragma once
#include "tier0/core.h"

#include <variant>

namespace FD_TIER(1)
{
template <typename... Types>
using variant = std::variant<Types...>;
using std::visit;
} // namespace FD_TIER(1)
