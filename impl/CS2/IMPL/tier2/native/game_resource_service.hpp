#pragma once

#include "tier2/core.h"

namespace FD_TIER2(native, cs2)
{
class game_entity_system;

// GameResourceServiceClientV001
class game_resource_service
{
  public:
    game_entity_system* get_game_entity_system();
};
}