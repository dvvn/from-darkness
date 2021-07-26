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
	this->Wait_for<input::wndproc>( );
	this->Wait_for<gui::menu>( );
}

void lock_cursor::Load( )
{
#ifndef CHEAT_GUI_TEST
	this->target_func_ = method_info::make_member_virtual(csgo_interfaces::get_shared( )->vgui_surface.get( ), 67);

	this->hook( );
	this->enable( );
#endif
}

string lock_cursor::Get_loaded_message( ) const
{
#ifndef CHEAT_GUI_TEST
		return service_base::Get_loaded_message( );
#else
	return Get_loaded_message_disabled( );
#endif
}

void lock_cursor::Callback( )
{
	if (gui::menu::get( ).visible( ) && !Target_instance( )->IsCursorVisible( ))
	{
		this->return_value_.set_original_called(true);
		Target_instance( )->UnlockCursor( );
	}
}
