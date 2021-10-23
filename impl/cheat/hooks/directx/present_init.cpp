#include "present.h"

#include "cheat/core/console.h"
#include "cheat/core/services_loader.h"
#include "cheat/gui/menu.h"
#include "cheat/netvars/config.h"

using namespace cheat;
using namespace hooks::directx;

present::present( )
{
	this->wait_for_service<gui::menu>( );
}

CHEAT_HOOK_PROXY_INIT_FN(present, TRUE)
CHEAT_HOOK_PROXY_TARGET_FN(present, TRUE, &csgo_interfaces::d3d_device, 17);

CHEAT_REGISTER_SERVICE(present);
