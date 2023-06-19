#pragma once

#include "core.h"

#include <variant>

namespace fd
{
template <typename... Types>
FD_WRAP_TOOL(variant, std::variant<Types...>);
using std::visit;
} // namespace fd

template <size_t Idx, typename... Types>
struct std::variant_alternative<Idx, fd::variant<Types...>> : variant_alternative<Idx, variant<Types...>>
{
};