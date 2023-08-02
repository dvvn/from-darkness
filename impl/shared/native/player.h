#pragma once
#include "internal/native_interface.h"
#include "partial/data_map.h"

namespace fd
{
struct native_library_info;
union native_entity_list;

FD_BIND_NATIVE_INTERFACE(C_CSPlayer, client);

union native_player
{
    native_player();

    native_player(native_library_info info);
    [[deprecated]] //
    native_player(native_entity_list list, size_t index);
    native_player(void *native);

    FD_NATIVE_INTERFACE(C_CSPlayer);
    partial_data_map<C_CSPlayer> data_map;
};
}