#pragma once

#include "hook/proxy.h"
#include "native/entity_instance.h"
#include "native/handle.h"

namespace fd::hooked::game_entity_system
{
template <class EntCache>
class on_add_entity : public basic_hook_callback
{
    EntCache* ent_cache_;

  public:
    on_add_entity(EntCache* ent_cache)
        : ent_cache_{ent_cache}
    {
    }

    void* operator()(auto const& original, native::entity_instance* instance, native::CBaseHandle handle)
    {
        ent_cache_->store(instance, handle);
        return original(instance, handle);
    }
};

template <class EntCache>
class on_remove_entity : public basic_hook_callback
{
    EntCache* ent_cache_;

  public:
    on_remove_entity(EntCache* ent_cache)
        : ent_cache_{ent_cache}
    {
    }

    void* operator()(auto const& original, native::entity_instance* instance, native::CBaseHandle handle)
    {
        ent_cache_->remove(instance, handle);
        return original(instance, handle);
    }
};
} // namespace fd::hooked::game_entity_system