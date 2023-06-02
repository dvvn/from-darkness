#pragma once

#include <cstdint>

namespace fd::valve
{
class entity_handle
{
    /*unsigned long*/ uint32_t value;

  public:
    uint32_t index() const;
};

}