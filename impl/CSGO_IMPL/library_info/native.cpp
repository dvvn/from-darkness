#include "library_info/native.h"
#include "memory/address.h"

namespace fd
{
interface_register* native_library_info::root_interface() const
{
    return **(interface_register***)get_absolute_address(function("CreateInterface"), 5, 6);
}
}