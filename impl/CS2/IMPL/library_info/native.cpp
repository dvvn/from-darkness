#include "library_info/native.h"
#include "memory/address.h"

namespace fd
{
auto native_library_info::root_interface() const -> interface_register*
{
    return *static_cast<interface_register**>(resolve_relative_address(function("CreateInterface"), 3, 7));
}
}