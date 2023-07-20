﻿#include "entity_list.h"
#include "player.h"
#include "library_info/native.h"

namespace fd
{

native_player::native_player(native_library_info info)
{
    construct_at(this, info.vtable("class C_CSPlayer"));
}

native_player::native_player(native_entity_list list, size_t index)
{
    construct_at(this, list.get_client_entity(index));
}
}