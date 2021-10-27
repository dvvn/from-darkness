#include "frame_stage_notify.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo_interfaces.h"
#include "cheat/core/services_loader.h"
#include "cheat/players/players_list.h"

using namespace cheat;
using namespace csgo;
using namespace hooks::client;

frame_stage_notify::frame_stage_notify( )
{
	this->wait_for_service<players_list>( );
}

nstd::address frame_stage_notify::get_target_method_impl( ) const
{
	return csgo_interfaces::get_ptr( )->client.vfunc(37);
}

service_impl::load_result frame_stage_notify::load_impl( ) noexcept
{
	CHEAT_LOAD_HOOK_PROXY;
}

CHEAT_REGISTER_SERVICE(frame_stage_notify);
