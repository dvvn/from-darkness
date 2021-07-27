#include "present.h"
#include "reset.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/gui/renderer.h"

using namespace cheat;
using namespace hooks;
using namespace directx;
using namespace utl;

present::present( )
{
}

bool present::Do_load( )
{
	target_func_ = method_info::make_member_virtual(csgo_interfaces::get_shared( )->d3d_device.get( ), 17);

	this->hook( );
	this->enable( );

	return 1;
}

void present::Callback(THIS_ CONST RECT*, CONST RECT*, HWND, CONST RGNDATA*)
{
	gui::renderer::get_shared( )->present(this->Target_instance( ));
}
