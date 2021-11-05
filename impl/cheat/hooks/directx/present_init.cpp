#include "present.h"

#include "cheat/core/console.h"
#include "cheat/core/services_loader.h"
#include "cheat/gui/menu.h"

#include <cppcoro/task.hpp>

using namespace cheat;
using namespace hooks::directx;

present_impl::present_impl( )
{
	this->add_dependency(gui::menu::get());
}

void* present_impl::get_target_method( ) const
{
	return csgo_interfaces::get( )->d3d_device.vfunc(17).ptr( );
}

auto present_impl::load_impl( ) noexcept -> load_result
{
	CHEAT_LOAD_HOOK_PROXY;
}

CHEAT_SERVICE_REGISTER(present);
