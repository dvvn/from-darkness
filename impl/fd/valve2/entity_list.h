#pragma once

#include <cstdint>

namespace fd::valve
{
void *get_client_entity(void *entity_list_interface, uint32_t index);
}