#pragma once
#include "internal/native_interface.h"
#include "partial/data_map.h"

namespace fd
{
struct native_library_info;
union native_entity_list;

FD_BIND_NATIVE_INTERFACE(C_BaseEntity, client);

union native_entity
{
    native_entity(native_library_info info);
    native_entity(native_entity_list list, size_t index);

    FD_NATIVE_INTERFACE(C_BaseEntity);
    partial_data_map<C_BaseEntity> data_map;
};
}