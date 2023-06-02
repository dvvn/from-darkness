#include "entity_list.h"

#include <fd/vfunc.h>

namespace fd::valve
{
vtable<void> entity_list_interface;

void *get_client_entity(uint32_t index)
{
    return (entity_list_interface)[3](index);
}
}