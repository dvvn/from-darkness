#include "draw model.h"

#include "cheat/core/csgo interfaces.h"

using namespace cheat;
using namespace csgo;
using namespace hooks;
using namespace studio_render;

draw_model::draw_model( )
: service_sometimes_skipped(
#if defined(CHEAT_GUI_TEST) || defined(CHEAT_NETVARS_UPDATING)
								true
#else
		false
#endif
							   )
{
}

nstd::address draw_model::get_target_method_impl( ) const
{
	return dhooks::_Pointer_to_virtual_class_table(csgo_interfaces::get_ptr( )->studio_renderer.get( ))[29];
}

void draw_model::callback(DrawModelResults_t* results, const DrawModelInfo_t& info,
						  matrix3x4_t*        bone_to_world,
						  float*              flex_weights, float*           flex_delayed_weights,
						  const Vector&       model_origin, DrawModelFlags_t flags)
{
}
