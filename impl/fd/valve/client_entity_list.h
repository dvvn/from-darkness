#pragma once

#include <fd/valve/client_entity.h>

// #include <fd/valve/vector.h>

namespace fd::valve
{
struct client_entity_listener
{
    virtual void OnEntityCreated(base_entity *pEntity) = 0;
    virtual void OnEntityDeleted(base_entity *pEntity) = 0;
};

struct client_entity_list
{
    virtual client_networkable *GetClientNetworkable(int entnum)                         = 0;
    virtual client_networkable *GetClientNetworkableFromHandle(base_handle hNetworkable) = 0;
    virtual client_unknown *GetClientUnknownFromHandle(base_handle hUnknown)             = 0;
    virtual client_entity *GetClientEntity(int entNum)                                   = 0;
    virtual client_entity *GetClientEntityFromHandle(base_handle hEnt)                   = 0;
    virtual int NumberOfEntities(bool IncludeNonNetworkable)                             = 0;
    virtual int GetHighestEntityIndex()                                                  = 0;
    virtual void SetMaxEntities(int maxEnts)                                             = 0;
    virtual int GetMaxEntities()                                                         = 0;

    void OnRemoveEntity(handle_entity *pEnt, base_handle handle) {}
    void OnAddEntity(handle_entity *pEnt, base_handle handle)    {}
    void RemovePVSNotifier(client_unknown *pUnknown)             {}

    /*private:
      vector<client_entity_listener *> EntityListeners;*/
};
} // namespace fd::valve