#pragma once
#include "algorithm/address.h"
#include "library_info/basic_interface_getter.h"

namespace fd
{
inline native_library_info::basic_interface_getter::basic_interface_getter(library_info const* linfo)
    : root_interface_{*static_cast<native::interface_register**>(resolve_relative_address(basic_function_getter{linfo}.create_interface(), 3, 7))}
{
}
} // namespace fd