#pragma once

#include <cstdint>

namespace fd::valve
{
struct entity_handle
{
    /*unsigned long*/ uint32_t value;
};

uint32_t get_entity_index(entity_handle handle);
}