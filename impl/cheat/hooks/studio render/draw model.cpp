#include "draw model.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/gui/menu.h"
#include "cheat/gui/user input.h"
#include "cheat/hooks/input/wndproc.h"
#include "cheat/players/players list.h"

using namespace cheat;
using namespace csgo;
using namespace hooks;
using namespace studio_render;
using namespace utl;

bool draw_model::Do_load( )
{
#ifdef CHEAT_GUI_TEST
	return false;
#else
	this->target_func_ = method_info::make_member_virtual(csgo_interfaces::get_shared( )->studio_renderer.get( ), 29);

	this->hook( );
	this->enable( );
	return true;
#endif
}

void draw_model::Callback(DrawModelResults_t* results, const DrawModelInfo_t& info,
						  matrix3x4_t* bone_to_world, float* flex_weights, float* flex_delayed_weights, const Vector& model_origin,
						  DrawModelFlags_t flags)
{
}
