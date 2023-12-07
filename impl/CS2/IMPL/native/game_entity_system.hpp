#pragma once

namespace fd::native::inline cs2
{
class C_BaseEntity;

class game_entity_system
{
  public:
    C_BaseEntity* get_base_entity(int index)
    {
        return 0;
    }

    int get_highest_entity_index()
    {
        int highestIdx = -1;

        // #ifdef _WIN32
        //     signatures::GetHighestEntityIndex.GetPtr().Call<void (*)(void*, int*)>(this, &highestIdx);
        // #elif __linux__
        //     highestIdx = signatures::GetHighestEntityIndex.GetPtr().Call<int (*)(void*)>(this);
        // #endif

        return highestIdx;
    }
};
}