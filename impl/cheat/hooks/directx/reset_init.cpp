#include "reset.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo_interfaces.h"
#include "cheat/core/services_loader.h"
#include "cheat/gui/imgui_context.h"

#include <cppcoro/task.hpp>

using namespace cheat;
using namespace hooks::directx;

reset_impl::reset_impl( )
{
	this->add_dependency(gui::imgui_context::get( ));
}

void* reset_impl::get_target_method( ) const
{
	return csgo_interfaces::get( )->d3d_device.vfunc(16).ptr( );
}

auto reset_impl::load_impl( ) noexcept -> load_result
{
	CHEAT_LOAD_HOOK_PROXY;
}

CHEAT_SERVICE_REGISTER(reset);
