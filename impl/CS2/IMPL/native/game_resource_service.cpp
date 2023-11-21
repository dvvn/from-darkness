#include "native/game_resource_service.hpp"

namespace fd::native
{
game_entity_system* game_resource_service::get_game_entity_system()
{
    return 0; // CPointer(this).GetField<CGameEntitySystem*>(platform::Constant(0x58, 0x50));
}
}