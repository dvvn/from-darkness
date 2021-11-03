#include "frame_stage_notify.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo_interfaces.h"
#include "cheat/core/services_loader.h"
#include "cheat/players/players_list.h"

#include <cppcoro/task.hpp>

using namespace cheat;
using namespace csgo;
using namespace hooks::client;

frame_stage_notify_impl::frame_stage_notify_impl( )
{
	this->add_dependency(players_list::get( ));
}

nstd::address frame_stage_notify_impl::get_target_method_impl( ) const
{
	return csgo_interfaces::get( )->client.vfunc(37);
}

auto frame_stage_notify_impl::load_impl( ) noexcept -> load_result
{
	CHEAT_LOAD_HOOK_PROXY;
}

CHEAT_SERVICE_REGISTER(frame_stage_notify);
