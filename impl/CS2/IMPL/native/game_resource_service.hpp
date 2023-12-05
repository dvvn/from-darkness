#pragma once



namespace fd::native::inline cs2
{
class game_entity_system;

// GameResourceServiceClientV001
class game_resource_service
{
  public:
    game_entity_system* get_game_entity_system();
};
}