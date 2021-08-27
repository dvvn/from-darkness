#include "lock cursor.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/gui/menu.h"
#include "cheat/players/players list.h"

using namespace cheat;
using namespace csgo;
using namespace hooks;
using namespace vgui_surface;

lock_cursor::lock_cursor( )
{
	this->add_service<gui::menu>( );
}

nstd::address lock_cursor::get_target_method_impl( ) const
{
	return dhooks::_Pointer_to_virtual_class_table(csgo_interfaces::get_ptr( )->vgui_surface.get( ))[67];
}

void lock_cursor::callback( )
{
	if (!object_instance->IsCursorVisible( ) && gui::menu::get_ptr( )->visible( ))
	{
		this->return_value_.set_original_called(true);
		object_instance->UnlockCursor( );
	}
}
