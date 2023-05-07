#include "entity_list.h"

#include <fd/vfunc.h>

namespace fd::valve
{
void *get_client_entity(void *entity_list_interface, uint32_t index)
{
    return vtable(entity_list_interface).call<void *>(3, index);
}
}