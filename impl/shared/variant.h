#pragma once

#include <variant>

namespace fd
{
template <typename... Types>
using variant = std::variant<Types...>;
using std::visit;
} // namespace fd
