#include "lock_cursor.h"

#include "cheat/core/console.h"
#include "cheat/core/services_loader.h"
#include "cheat/gui/menu.h"

using namespace cheat;
using namespace hooks::vgui_surface;

lock_cursor_impl::lock_cursor_impl( )
{
	this->add_dependency(gui::menu::get());
}

nstd::address lock_cursor_impl::get_target_method_impl( ) const
{
	return csgo_interfaces::get( )->vgui_surface.vfunc(67);
}

basic_service::load_result lock_cursor_impl::load_impl( ) noexcept
{
	CHEAT_LOAD_HOOK_PROXY;
}

CHEAT_SERVICE_REGISTER(lock_cursor);
