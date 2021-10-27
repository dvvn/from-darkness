#include "reset.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo_interfaces.h"
#include "cheat/core/services_loader.h"
#include "cheat/gui/imgui_context.h"

using namespace cheat;
using namespace hooks::directx;

reset::reset( )
{
	this->wait_for_service<gui::imgui_context>( );
}

nstd::address reset::get_target_method_impl( ) const
{
	return csgo_interfaces::get_ptr( )->d3d_device.vfunc(16);
}

service_impl::load_result reset::load_impl( ) noexcept
{
	CHEAT_LOAD_HOOK_PROXY;
}

CHEAT_REGISTER_SERVICE(reset);
