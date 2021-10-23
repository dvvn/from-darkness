#include "lock_cursor.h"

#include "cheat/core/console.h"
#include "cheat/core/services_loader.h"
#include "cheat/gui/menu.h"
#include "cheat/netvars/config.h"

using namespace cheat;
using namespace hooks::vgui_surface;

lock_cursor::lock_cursor( )
{
	this->wait_for_service<gui::menu>( );
}

CHEAT_HOOK_PROXY_INIT_FN(lock_cursor, CHEAT_MODE_INGAME)
CHEAT_HOOK_PROXY_TARGET_FN(lock_cursor, CHEAT_MODE_INGAME, &csgo_interfaces::vgui_surface, 67);

CHEAT_REGISTER_SERVICE(lock_cursor);
