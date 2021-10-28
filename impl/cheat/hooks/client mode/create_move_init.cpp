#include "create_move.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo_interfaces.h"
#include "cheat/core/services_loader.h"
#include "cheat/players/players_list.h"

using namespace cheat;
using namespace csgo;
using namespace hooks::client_mode;

create_move_impl::create_move_impl( )
{
	this->add_dependency(players_list::get( ));
	this->get_address_of_return_address( ).emplace( );
}

nstd::address create_move_impl::get_target_method_impl( ) const
{
	return csgo_interfaces::get( )->client_mode.vfunc(24);
}

basic_service::load_result create_move_impl::load_impl( ) noexcept
{
	CHEAT_LOAD_HOOK_PROXY;
}

CHEAT_SERVICE_REGISTER(create_move);
