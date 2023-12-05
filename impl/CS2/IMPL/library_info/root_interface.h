#pragma once
#include "memory/address.h"
#include "library_info_native.h"

namespace fd
{
inline native::interface_register* native_library_info::root_interface() const
{
    auto const ptr = *static_cast<native::interface_register**>(resolve_relative_address(function("CreateInterface"), 3, 7));
    return ptr;
}
}