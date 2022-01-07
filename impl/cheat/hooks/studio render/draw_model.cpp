module;

#include "cheat/hooks/base_includes.h"
#include "cheat/players/player_includes.h"

module cheat.hooks.studio_render:draw_model;
import cheat.players;

using namespace cheat;
using namespace csgo;
using namespace hooks::studio_render;

draw_model::draw_model( )
{
	this->add_dependency(players_list::get( ));
}

void* draw_model::get_target_method( ) const
{
	return csgo_interfaces::get( )->studio_renderer.vfunc(29).ptr( );
}

void draw_model::callback(DrawModelResults_t* results, const DrawModelInfo_t& info,
						  matrix3x4_t* bone_to_world,
						  float* flex_weights, float* flex_delayed_weights,
						  const Vector& model_origin, DrawModelFlags_t flags)
{
}

CHEAT_SERVICE_REGISTER_GAME(draw_model);


