#pragma once
#include <fd/valve/client_side/collideable.h>
#include <fd/valve/entity_handle.h>

namespace fd::valve::client_side
{
struct networkable;
struct entity;
struct renderable;
struct thinkable;

struct unknown : entity_handle
{
    virtual collideable *GetCollideable()                             = 0;
    virtual networkable *GetClientNetworkable()                       = 0;
    virtual renderable *GetClientRenderable()                         = 0;
    virtual entity *GetIClientEntity()                                = 0;
    virtual entity *GetBaseEntity()                                   = 0;
    virtual thinkable *GetClientThinkable()                           = 0;
    // virtual IClientModelRenderable*  GetClientModelRenderable() = 0;
    virtual /* IClientAlphaProperty */ void *GetClientAlphaProperty() = 0;
};
}