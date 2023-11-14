#include "library_info/native.h"
#include "memory/address.h"

namespace fd
{
interface_register* native_library_info::root_interface() const
{
    return *static_cast<interface_register**>(resolve_relative_address(function("CreateInterface"), 3, 7));
}
}