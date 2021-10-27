#include "draw_model.h"

#include "cheat/core/console.h"
#include "cheat/core/services_loader.h"
#include "cheat/players/players_list.h"

using namespace cheat;
using namespace hooks::studio_render;

draw_model::draw_model( )
{
	this->wait_for_service<players_list>( );
}

nstd::address draw_model::get_target_method_impl( ) const
{
	return csgo_interfaces::get_ptr( )->studio_renderer.vfunc(29);
}

service_impl::load_result draw_model::load_impl( ) noexcept
{
	CHEAT_LOAD_HOOK_PROXY;
}

CHEAT_REGISTER_SERVICE(draw_model);
