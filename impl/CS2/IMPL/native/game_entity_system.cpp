#include "native/game_entity_system.hpp"

namespace fd::native
{
C_BaseEntity* game_entity_system::get_base_entity(int index)
{
    return 0;
}

int game_entity_system::get_highest_entity_index()
{
    int highestIdx = -1;

    // #ifdef _WIN32
    //     signatures::GetHighestEntityIndex.GetPtr().Call<void (*)(void*, int*)>(this, &highestIdx);
    // #elif __linux__
    //     highestIdx = signatures::GetHighestEntityIndex.GetPtr().Call<int (*)(void*)>(this);
    // #endif

    return highestIdx;
}
}