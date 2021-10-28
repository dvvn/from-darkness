#include "lock_cursor.h"

#include "cheat/gui/menu.h"

#include "cheat/csgo/ISurface.hpp"

using namespace cheat;
using namespace hooks::vgui_surface;

void lock_cursor_impl::callback( )
{
	if (!object_instance->IsCursorVisible( ) && gui::menu::get( )->visible( ))
	{
		return_value_.set_original_called(true);
		object_instance->UnlockCursor( );
	}
}
