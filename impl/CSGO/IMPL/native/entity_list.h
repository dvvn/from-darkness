#pragma once

#include "internal/native_interface.h"

namespace fd
{
struct native_library_info;

FD_BIND_NATIVE_INTERFACE(CClientEntityList, client);
FD_BIND_NATIVE_INTERFACE(IClientEntityList, client);

union native_entity_list
{
    native_entity_list(native_library_info info);

    FD_NATIVE_INTERFACE(IClientEntityList);
    function<3, void *, uint32_t /*index*/> get_client_entity;
    function<8, int32_t> max_entities;

    
};
}