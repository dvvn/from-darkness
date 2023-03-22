#pragma once

#include <fd/valve/client_side/entity.h>

namespace fd::valve::client_side
{
struct entity_list
{
    virtual networkable *GetClientNetworkable(int entnum)                    = 0;
    virtual networkable *GetClientNetworkableFromHandle(handle hNetworkable) = 0;
    virtual unknown *GetClientUnknownFromHandle(handle hUnknown)             = 0;
    virtual entity *GetClientEntity(int entNum)                              = 0;
    virtual entity *GetClientEntityFromHandle(handle hEnt)                   = 0;
    virtual int NumberOfEntities(bool IncludeNonNetworkable)                 = 0;
    virtual int GetHighestEntityIndex()                                      = 0;
    virtual void SetMaxEntities(int maxEnts)                                 = 0;
    virtual int GetMaxEntities()                                             = 0;
};
}