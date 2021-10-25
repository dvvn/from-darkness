#include "frame_stage_notify.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo_interfaces.h"
#include "cheat/core/services_loader.h"
#include "cheat/netvars/config.h"
#include "cheat/players/players_list.h"

using namespace cheat;
using namespace csgo;
using namespace hooks::client;

frame_stage_notify::frame_stage_notify( )
{
	this->wait_for_service<players_list>( );
}

CHEAT_HOOK_PROXY_INIT_FN(frame_stage_notify, CHEAT_MODE_INGAME)
CHEAT_HOOK_PROXY_TARGET_FN(frame_stage_notify, CHEAT_MODE_INGAME, &csgo_interfaces::client, 37);

CHEAT_REGISTER_SERVICE(frame_stage_notify);
