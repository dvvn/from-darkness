#pragma once

#include "native/entity_instance.h"

namespace fd
{
struct entity
{
    native::entity_instance* instance;

    bool equal(native::entity_instance const* other) const
    {
        return instance == other;
    }
};
} // namespace fd
