#include "reset.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo_interfaces.h"
#include "cheat/core/services_loader.h"
#include "cheat/gui/imgui_context.h"
#include "cheat/netvars/config.h"

using namespace cheat;
using namespace hooks::directx;

reset::reset( )
{
	this->wait_for_service<gui::imgui_context>( );
}

CHEAT_HOOK_PROXY_INIT_FN(reset, TRUE)
CHEAT_HOOK_PROXY_TARGET_FN(reset, TRUE, &csgo_interfaces::d3d_device, 16);
CHEAT_REGISTER_SERVICE(reset);
