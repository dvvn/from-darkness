#pragma once
#include <cstdint>

namespace fd::native::inline cs2
{
class game_entity_system;

// GameResourceServiceClientV001
class game_resource_service
{
  public:
    game_entity_system* get_game_entity_system() const
    {
        return reinterpret_cast<game_entity_system*>(reinterpret_cast<uintptr_t>(this) + 0x58);
    }
};
} // namespace fd::native::inline cs2