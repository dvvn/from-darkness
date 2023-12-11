#pragma once

#include "hook/proxy.h"
#include "native/entity_instance.h"
#include "native/handle.h"

namespace fd::hooked::game_entity_system
{
template <class Reserved>
struct on_add_entity : basic_hook_callback
{
    void* operator()(auto const& original, native::entity_instance* instance, native::CBaseHandle handle)
    {
        // reserved
        return original(instance, handle);
    }
};

template <class Reserved>
struct on_remove_entity : basic_hook_callback
{
    void* operator()(auto const& original, native::entity_instance* instance, native::CBaseHandle handle)
    {
        // reserved
        return original(instance, handle);
    }
};
} // namespace fd::hooked::game_entity_system