#include "draw_model.h"

#include "cheat/core/console.h"
#include "cheat/core/services_loader.h"
#include "cheat/netvars/config.h"
#include "cheat/players/players_list.h"

using namespace cheat;
using namespace hooks::studio_render;

draw_model::draw_model( )
{
	this->wait_for_service<players_list>( );
}

CHEAT_HOOK_PROXY_INIT_FN(draw_model, CHEAT_MODE_INGAME)
CHEAT_HOOK_PROXY_TARGET_FN(draw_model, CHEAT_MODE_INGAME, &csgo_interfaces::studio_renderer, 29);

CHEAT_REGISTER_SERVICE(draw_model);
