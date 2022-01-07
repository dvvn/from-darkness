module;

#include "cheat/hooks/base_includes.h"

module cheat.hooks.vgui_surface:lock_cursor;
import cheat.gui.menu;

using namespace cheat;
using namespace hooks::vgui_surface;

lock_cursor::lock_cursor( )
{
	this->add_dependency(gui::menu::get( ));
}

void* lock_cursor::get_target_method( ) const
{
	return csgo_interfaces::get( )->vgui_surface.vfunc(67).ptr( );
}

void lock_cursor::callback( )
{
	auto inst = this->get_object_instance( );
	if (!inst->IsCursorVisible( ) && gui::menu::get( )->visible( ))
	{
		this->store_return_value( );
		inst->UnlockCursor( );
	}
}

CHEAT_SERVICE_REGISTER_GAME(lock_cursor);