#include "reset.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/gui/imgui context.h"

#include <backends/imgui_impl_dx9.h>

using namespace cheat;
using namespace hooks;
using namespace directx;

reset::reset( )
{
}

nstd::address reset::get_target_method_impl( ) const
{
	return dhooks::_Pointer_to_virtual_class_table(csgo_interfaces::get_ptr( )->d3d_device.get( ))[16];
}

void reset::callback(D3DPRESENT_PARAMETERS*)
{
	ImGui_ImplDX9_InvalidateDeviceObjects( );
}
