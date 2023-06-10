#include "entity_list.h"

#include <fd/vtable.h>

namespace fd::valve
{
vtable<void> entity_list_interface;

void *get_client_entity(uint32_t index)
{
    return invoke(entity_list_interface[3], index);
}
}