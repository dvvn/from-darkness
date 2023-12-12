#pragma once
#include "algorithm/address.h"
#include "library_info/interface.h"

namespace fd::detail
{
inline auto library_interface_getter<>::root_interface() const -> native::interface_register*
{
    auto const ptr = *static_cast<native::interface_register**>(resolve_relative_address(create_interface(), 3, 7));
    return ptr;
}
} // namespace fd::detail