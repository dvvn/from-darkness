#include "reset.h"

#include "cheat/core/csgo interfaces.h"

using namespace cheat;
using namespace hooks;
using namespace directx;
using namespace utl;

reset::reset( )
{
}

bool reset::Do_load( )
{
	this->target_func_ = method_info::make_member_virtual(csgo_interfaces::get_ptr( )->d3d_device.get( ), 16);

	this->hook( );
	this->enable( );

	return true;
}

void reset::Callback(D3DPRESENT_PARAMETERS*)
{
	ImGui_ImplDX9_InvalidateDeviceObjects( );
}
