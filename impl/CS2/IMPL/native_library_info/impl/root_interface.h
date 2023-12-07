#pragma once
#include "algorithm/address.h"
#include "native_library_info.h"

namespace fd
{
inline native::interface_register* native_library_info::basic_interface_getter::root_interface() const
{
    auto const ptr = *static_cast<native::interface_register**>(resolve_relative_address(create_interface(), 3, 7));
    return ptr;
}
}