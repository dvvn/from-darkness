#include "lock cursor.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/gui/menu.h"
#include "cheat/gui/user input.h"
#include "cheat/hooks/input/wndproc.h"
#include "cheat/players/players list.h"

using namespace cheat;
using namespace csgo;
using namespace hooks;
using namespace vgui_surface;
using namespace utl;

lock_cursor::lock_cursor( )
{
}

bool lock_cursor::Do_load( )
{
#ifdef CHEAT_GUI_TEST
	return false;
#else
	this->target_func_ = method_info::make_member_virtual(csgo_interfaces::get_shared( )->vgui_surface.get( ), 67);

	this->hook( );
	this->enable( );
	return true;
#endif
}

void lock_cursor::Callback( )
{
	if (!Target_instance( )->IsCursorVisible( ) && gui::menu::get_shared( )->visible( ))
	{
		this->return_value_.set_original_called(true);
		Target_instance( )->UnlockCursor( );
	}
}
