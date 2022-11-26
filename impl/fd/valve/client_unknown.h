#pragma once

#include <fd/valve/base_handle.h>

namespace fd::valve
{
    struct collideable;
    struct client_networkable;
    struct client_thinkable;
    struct base_entity;
    struct client_entity;

    struct client_unknown : handle_entity
    {
        virtual collideable* GetCollideable()                             = 0;
        virtual client_networkable* GetClientNetworkable()                = 0;
        virtual client_renderable* GetClientRenderable()                  = 0;
        virtual client_entity* GetIClientEntity()                         = 0;
        virtual base_entity* GetBaseEntity()                              = 0;
        virtual client_thinkable* GetClientThinkable()                    = 0;
        // virtual IClientModelRenderable*  GetClientModelRenderable() = 0;
        virtual /* IClientAlphaProperty */ void* GetClientAlphaProperty() = 0;
    };
} // namespace fd::valve