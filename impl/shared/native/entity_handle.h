#pragma once

#include <cstdint>

namespace fd
{
class native_entity_handle
{
    /*unsigned long*/ uint32_t value;

  public:
    uint32_t index() const;
};

}