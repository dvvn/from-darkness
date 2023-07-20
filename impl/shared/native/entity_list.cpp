#include "entity_list.h"
#include "library_info/native.h"

namespace fd
{
native_entity_list::native_entity_list(native_library_info info)
{
    construct_at(this, info.interface("VClientEntityList"));
}
}