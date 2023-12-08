#pragma once

namespace fd::native::inline cs2
{
class game_entity_system;

// GameResourceServiceClientV001
class game_resource_service
{
  public:
    game_entity_system* get_game_entity_system()
    {
        return 0; // CPointer(this).GetField<CGameEntitySystem*>(platform::Constant(0x58, 0x50));
    }
};
}