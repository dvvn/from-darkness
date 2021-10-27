#include "lock_cursor.h"

#include "cheat/core/console.h"
#include "cheat/core/services_loader.h"
#include "cheat/gui/menu.h"

using namespace cheat;
using namespace hooks::vgui_surface;

lock_cursor::lock_cursor( )
{
	this->wait_for_service<gui::menu>( );
}

nstd::address lock_cursor::get_target_method_impl( ) const
{
	return csgo_interfaces::get_ptr( )->vgui_surface.vfunc(67);
}

service_impl::load_result lock_cursor::load_impl( ) noexcept
{
	CHEAT_LOAD_HOOK_PROXY;
}

CHEAT_REGISTER_SERVICE(lock_cursor);
