module;

#include "cheat/hooks/base_includes.h"
#include "cheat/players/player_includes.h"

module cheat.hooks.studio_render:draw_model;
import cheat.players;
import nstd.mem.address;

using namespace cheat;
using namespace csgo;
using namespace hooks::studio_render;

draw_model::draw_model( ) = default;

void draw_model::construct( ) noexcept
{
	//this->set_target_method(this->deps( ).get<csgo_interfaces>( ).studio_renderer.vfunc(29));
	const nstd::mem::basic_address vtable_holder = IStudioRender::get_ptr( );
	this->set_target_method(vtable_holder.deref<1>( )[29]);
}

void draw_model::callback(DrawModelResults_t * results, const DrawModelInfo_t & info,
						  matrix3x4_t * bone_to_world,
						  float* flex_weights, float* flex_delayed_weights,
						  const Vector & model_origin, DrawModelFlags_t flags)
{
}

