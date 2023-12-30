#pragma once
#include "algorithm/address.h"
#include "library_info/basic_interface_getter.h"
#include "library_info/function_getter.h"

namespace fd::detail
{
inline native_library_interface_getter::native_library_interface_getter(library_info const* linfo)
    : root_interface_{*static_cast<native::interface_register**>(resolve_relative_address(native_library_function_getter{linfo}.create_interface(), 3, 7))}
{
}
} // namespace fd::detail