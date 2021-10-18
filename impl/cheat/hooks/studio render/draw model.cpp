#include "draw model.h"

#include "cheat/core/services loader.h"
#include "cheat/core/csgo interfaces.h"
#include "cheat/core/console.h"

#include "cheat/netvars/config.h"

#include "cheat/players/players_list.h"

using namespace cheat;
using namespace csgo;
using namespace hooks;
using namespace studio_render;

draw_model::draw_model( )
{
	this->wait_for_service<players_list>( );
}

CHEAT_HOOK_PROXY_INIT_FN(draw_model, CHEAT_MODE_INGAME)
CHEAT_HOOK_PROXY_TARGET_FN(draw_model, CHEAT_MODE_INGAME, &csgo_interfaces::studio_renderer, 29);

void draw_model::callback(DrawModelResults_t* results, const DrawModelInfo_t& info,
						  matrix3x4_t* bone_to_world,
						  float* flex_weights, float* flex_delayed_weights,
						  const Vector& model_origin, DrawModelFlags_t flags)
{
#if !CHEAT_MODE_INGAME
	CHEAT_CALL_BLOCKER
#else

#endif
}

CHEAT_REGISTER_SERVICE(draw_model);
