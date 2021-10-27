#include "create_move.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo_interfaces.h"
#include "cheat/core/services_loader.h"
#include "cheat/players/players_list.h"

using namespace cheat;
using namespace csgo;
using namespace hooks::client_mode;

create_move::create_move( )
{
	this->wait_for_service<players_list>( );
	this->get_address_of_return_address( ).emplace( );
}

nstd::address create_move::get_target_method_impl( ) const
{
	return csgo_interfaces::get_ptr( )->client_mode.vfunc(24);
}

service_impl::load_result create_move::load_impl( ) noexcept
{
	CHEAT_LOAD_HOOK_PROXY;
}

CHEAT_REGISTER_SERVICE(create_move);
