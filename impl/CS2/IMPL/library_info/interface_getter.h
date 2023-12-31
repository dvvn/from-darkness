#pragma once
#include "algorithm/address.h"
#include "library_info/basic_interface_getter.h"
#include "library_info/function_getter.h"

namespace fd::detail
{
inline native::interface_register* native_library_interface_getter::get_root_interface(library_info const* linfo)
{
    native_library_function_getter const fn_getter{linfo};
    auto const create_fn = fn_getter.create_interface();

    auto const addr = static_cast<native::interface_register**>(resolve_relative_address(create_fn, 3, 7));
    return *addr;
}
} // namespace fd::detail