#include "create_move.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo_interfaces.h"
#include "cheat/core/services_loader.h"
#include "cheat/players/players_list.h"

#include <cppcoro/task.hpp>

using namespace cheat;
using namespace csgo;
using namespace hooks::client_mode;

create_move_impl::create_move_impl( )
{
	this->add_dependency(players_list::get( ));
}

void* create_move_impl::get_target_method( ) const
{
	return csgo_interfaces::get( )->client_mode.vfunc(24).ptr( );
}

auto create_move_impl::load_impl( ) noexcept -> load_result
{
	CHEAT_LOAD_HOOK_PROXY;
}

CHEAT_SERVICE_REGISTER(create_move);
