#include "reset.h"

#include "cheat/core/console.h"
#include "cheat/core/services loader.h"
#include "cheat/core/csgo interfaces.h"
#include "cheat/gui/imgui context.h"
#include "cheat/netvars/config.h"

#include <backends/imgui_impl_dx9.h>

using namespace cheat;
using namespace hooks;
using namespace directx;

reset::reset()
{
	this->wait_for_service<gui::imgui_context>( );
}

CHEAT_HOOK_PROXY_INIT_FN(reset, TRUE)
CHEAT_HOOK_PROXY_TARGET_FN(reset, TRUE, &csgo_interfaces::d3d_device, 16);

void reset::callback(D3DPRESENT_PARAMETERS*)
{
	ImGui_ImplDX9_InvalidateDeviceObjects( );
}

CHEAT_REGISTER_SERVICE(reset);
