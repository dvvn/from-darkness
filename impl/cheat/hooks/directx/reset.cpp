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
	this->hook( );
	this->enable( );

	return true;
}

utl::address reset::get_target_method_impl( ) const
{
	return _Pointer_to_virtual_class_table(csgo_interfaces::get_ptr( )->d3d_device.get( ))[16];
}

void reset::callback(D3DPRESENT_PARAMETERS*)
{
	ImGui_ImplDX9_InvalidateDeviceObjects( );
}
