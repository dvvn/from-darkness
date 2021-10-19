#include "lock cursor.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo interfaces.h"
#include "cheat/core/services loader.h"

#include "cheat/gui/menu.h"
#include "cheat/netvars/config.h"
#include "cheat/csgo/ISurface.hpp"

using namespace cheat;
using namespace csgo;
using namespace hooks;
using namespace vgui_surface;

lock_cursor::lock_cursor()
{
	this->wait_for_service<gui::menu>( );
}

CHEAT_HOOK_PROXY_INIT_FN(lock_cursor, CHEAT_MODE_INGAME)
CHEAT_HOOK_PROXY_TARGET_FN(lock_cursor, CHEAT_MODE_INGAME, &csgo_interfaces::vgui_surface, 67);

void lock_cursor::callback()
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

CHEAT_REGISTER_SERVICE(lock_cursor);
