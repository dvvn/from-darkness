#include "draw model.h"

#include "cheat/core/services loader.h"
#include "cheat/core/csgo interfaces.h"
#include "cheat/core/console.h"

#include "cheat/netvars/config.h"

#include "cheat/players/players list.h"

using namespace cheat;
using namespace csgo;
using namespace hooks;
using namespace studio_render;

draw_model::draw_model()
{
	this->wait_for_service<players_list>( );
}

nstd::address draw_model::get_target_method_impl() const
{
	return dhooks::_Pointer_to_virtual_class_table(csgo_interfaces::get_ptr( )->studio_renderer.get( ))[29];
}

CHEAT_SERVICE_HOOK_PROXY_IMPL_SIMPLE(draw_model)

void draw_model::callback(DrawModelResults_t* results, const DrawModelInfo_t& info,
						  matrix3x4_t* bone_to_world,
						  float* flex_weights, float* flex_delayed_weights,
						  const Vector& model_origin, DrawModelFlags_t flags)
{
#if !CHEAT_SERVICE_INGAME
	runtime_assert("Skipped but called");
#pragma message(__FUNCTION__ ": skipped")
#else

#endif
}

CHEAT_REGISTER_SERVICE(draw_model);
