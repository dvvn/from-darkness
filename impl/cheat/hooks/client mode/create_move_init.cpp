#include "create_move.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo_interfaces.h"
#include "cheat/core/services_loader.h"
#include "cheat/netvars/config.h"
#include "cheat/players/players_list.h"

using namespace cheat;
using namespace csgo;
using namespace hooks::client_mode;

create_move::create_move( )
{
	this->wait_for_service<players_list>( );
	this->get_address_of_return_address( ).emplace( );
}

CHEAT_HOOK_PROXY_INIT_FN(create_move, CHEAT_MODE_INGAME)
CHEAT_HOOK_PROXY_TARGET_FN(create_move, CHEAT_MODE_INGAME, &cheat::csgo_interfaces::client_mode, 24);

CHEAT_REGISTER_SERVICE(create_move);
