#pragma once

#include <fd/valve/client_entity.h>

namespace fd::valve
{
    struct client_entity_list
    {
        virtual client_networkable* GetClientNetworkable(int entnum)       = 0;
        virtual void* vtablepad0x1()                                       = 0;
        virtual void* vtablepad0x2()                                       = 0;
        virtual client_entity* GetClientEntity(int entNum)                 = 0;
        virtual client_entity* GetClientEntityFromHandle(base_handle hEnt) = 0;
        virtual int NumberOfEntities(bool IncludeNonNetworkable)           = 0;
        virtual int GetHighestEntityIndex()                                = 0;
        virtual void SetMaxEntities(int maxEnts)                           = 0;
        virtual int GetMaxEntities()                                       = 0;
    };
} // namespace fd::valve