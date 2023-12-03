#include "tier1/memory/address.h"
#include "tier2/library_info/native.h"

namespace FD_TIER(2)
{
auto native_library_info::root_interface() const -> interface_register*
{
    auto const ptr = *static_cast<interface_register**>(resolve_relative_address(function("CreateInterface"), 3, 7));
    return ptr;
}
}