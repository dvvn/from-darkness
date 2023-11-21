#pragma once

namespace fd::native::inline cs2
{
class C_BaseEntity;

class game_entity_system
{
  public:
    C_BaseEntity* get_base_entity(int index);
    int get_highest_entity_index();
};
}