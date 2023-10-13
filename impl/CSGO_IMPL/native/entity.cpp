#include "entity.h"
#include "entity_list.h"
#include "library_info/native.h"

namespace fd
{
native_entity::native_entity(native_library_info const info)
{
    construct_at(this, info.vtable("class C_BaseEntity"));
}

native_entity::native_entity(native_entity_list const list, size_t const index)
{
    construct_at(this, list.get_client_entity(index));
}
}