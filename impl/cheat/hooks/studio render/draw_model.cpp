module;

#include "cheat/hooks/base_includes.h"
#include "cheat/players/player_includes.h"

module cheat.hooks.studio_render:draw_model;
import cheat.players;

using namespace cheat;
using namespace csgo;
using namespace hooks::studio_render;

draw_model::draw_model( ) = default;

void draw_model::construct( ) noexcept
{
	this->deps( ).add<csgo_interfaces>( );
	this->deps( ).add<players_list>( );
}

bool draw_model::load( ) noexcept
{
	this->set_target_method(this->deps( ).get<csgo_interfaces>( ).studio_renderer.vfunc(29).ptr( ));
	return hook_base::load( );
}

void draw_model::callback(DrawModelResults_t * results, const DrawModelInfo_t & info,
						  matrix3x4_t * bone_to_world,
						  float* flex_weights, float* flex_delayed_weights,
						  const Vector & model_origin, DrawModelFlags_t flags)
{
}

