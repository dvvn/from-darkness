#include "lock_cursor.h"

#include "cheat/gui/menu.h"

#include "cheat/csgo/ISurface.hpp"

using namespace cheat;
using namespace hooks::vgui_surface;

void lock_cursor::callback( )
{
#if !CHEAT_MODE_INGAME
	CHEAT_CALL_BLOCKER
#else
	if (!object_instance->IsCursorVisible( ) && gui::menu::get_ptr( )->visible( ))
	{
		return_value_.set_original_called(true);
		object_instance->UnlockCursor( );
	}
#endif
}
