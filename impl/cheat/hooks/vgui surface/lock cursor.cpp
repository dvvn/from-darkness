#include "lock cursor.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo interfaces.h"
#include "cheat/core/services loader.h"

#include "cheat/gui/menu.h"
#include "cheat/netvars/config.h"
#include "cheat/sdk/ISurface.hpp"

using namespace cheat;
using namespace csgo;
using namespace hooks;
using namespace vgui_surface;

lock_cursor::lock_cursor()
{
	this->wait_for_service<gui::menu>( );
}

CHEAT_SERVICE_HOOK_PROXY_IMPL_SIMPLE(lock_cursor)

nstd::address lock_cursor::get_target_method_impl() const
{
	return dhooks::_Pointer_to_virtual_class_table(csgo_interfaces::get_ptr( )->vgui_surface.get( ))[67];
}

void lock_cursor::callback()
{
#if !CHEAT_SERVICE_INGAME
	runtime_assert("Skipped but called");
#pragma message(__FUNCTION__ ": skipped")
#else
	if (!object_instance->IsCursorVisible( ) && gui::menu::get_ptr( )->visible( ))
	{
		return_value_.set_original_called(true);
		object_instance->UnlockCursor( );
	}
#endif
}

CHEAT_REGISTER_SERVICE(lock_cursor);
