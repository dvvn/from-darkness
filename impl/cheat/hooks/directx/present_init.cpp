#include "present.h"

#include "cheat/core/console.h"
#include "cheat/core/services_loader.h"
#include "cheat/gui/menu.h"

using namespace cheat;
using namespace hooks::directx;

present::present( )
{
	this->wait_for_service<gui::menu>( );
}

nstd::address present::get_target_method_impl( ) const
{
	return csgo_interfaces::get_ptr( )->d3d_device.vfunc(17);
}

service_impl::load_result present::load_impl( ) noexcept
{
	CHEAT_LOAD_HOOK_PROXY;
}

CHEAT_REGISTER_SERVICE(present);
