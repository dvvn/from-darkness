#include "reset.h"

#include <backends/imgui_impl_dx9.h>

using namespace cheat;
using namespace hooks::directx;

void reset::callback(D3DPRESENT_PARAMETERS*)
{
	ImGui_ImplDX9_InvalidateDeviceObjects( );
}
