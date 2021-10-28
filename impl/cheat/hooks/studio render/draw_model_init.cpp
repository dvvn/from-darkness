#include "draw_model.h"

#include "cheat/core/console.h"
#include "cheat/core/services_loader.h"
#include "cheat/players/players_list.h"

using namespace cheat;
using namespace hooks::studio_render;

draw_model_impl::draw_model_impl( )
{
	this->add_dependency(players_list::get( ));
}

nstd::address draw_model_impl::get_target_method_impl( ) const
{
	return csgo_interfaces::get( )->studio_renderer.vfunc(29);
}

basic_service::load_result draw_model_impl::load_impl( ) noexcept
{
	CHEAT_LOAD_HOOK_PROXY;
}

CHEAT_SERVICE_REGISTER(draw_model);
