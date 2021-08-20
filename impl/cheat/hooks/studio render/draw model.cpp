#include "draw model.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/gui/menu.h"
#include "cheat/players/players list.h"

using namespace cheat;
using namespace csgo;
using namespace hooks;
using namespace studio_render;
using namespace utl;



utl::address draw_model::get_target_method_impl( ) const
{
	return _Pointer_to_virtual_class_table(csgo_interfaces::get_ptr( )->studio_renderer.get( ))[29];
}

void draw_model::callback(DrawModelResults_t* results, const DrawModelInfo_t& info,
						  matrix3x4_t*        bone_to_world,
						  float*              flex_weights, float*           flex_delayed_weights,
						  const Vector&       model_origin, DrawModelFlags_t flags)
{
}
