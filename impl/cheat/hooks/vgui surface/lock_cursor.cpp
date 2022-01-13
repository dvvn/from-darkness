module;

#include "cheat/hooks/base_includes.h"

module cheat.hooks.vgui_surface:lock_cursor;
import cheat.gui.menu;

using namespace cheat;
using namespace hooks::vgui_surface;

lock_cursor::lock_cursor( ) = default;

void lock_cursor::load_async( ) noexcept
{
	this->deps( ).add<gui::menu>( );
}

void* lock_cursor::get_target_method( ) const
{
	return services_loader::get( ).deps( ).get<csgo_interfaces>( ).vgui_surface.vfunc(67).ptr( );
}

void lock_cursor::callback( )
{
	auto inst = this->get_object_instance( );
	if (!inst->IsCursorVisible( ) && this->deps( ).get<gui::menu>( ).visible( ))
	{
		this->store_return_value( );
		inst->UnlockCursor( );
	}
}