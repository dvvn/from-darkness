module;

#include "cheat/hooks/base_includes.h"

module cheat.hooks.vgui_surface:lock_cursor;
import cheat.gui;

using namespace cheat;
using namespace hooks::vgui_surface;

lock_cursor::lock_cursor( ) = default;

void lock_cursor::construct( ) noexcept
{
	this->deps( ).add<csgo_interfaces>( );
	this->deps( ).add<gui::menu>( );
}

bool lock_cursor::load( ) noexcept
{
	this->set_target_method(this->deps( ).get<csgo_interfaces>( ).vgui_surface.vfunc(67).ptr( ));
	return hook_base::load( );
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