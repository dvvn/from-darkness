#pragma once

#include "interface.h"

namespace fd
{
FD_BIND_NATIVE_INTERFACE(CClientEntityList, client);
FD_BIND_NATIVE_INTERFACE(IClientEntityList, client);

union native_entity_list
{
    FD_NATIVE_INTERFACE(IClientEntityList);
    function<3, void *, uint32_t /*index*/> get_client_entity;
    function<8, int32_t> max_entities;
};
}