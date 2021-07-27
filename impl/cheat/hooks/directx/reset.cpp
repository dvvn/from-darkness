#include "reset.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/gui/renderer.h"

using namespace cheat;
using namespace hooks;
using namespace directx;
using namespace utl;

reset::reset( )
{
}

bool reset::Do_load( )
{
	this->target_func_ = method_info::make_member_virtual(csgo_interfaces::get_shared( )->d3d_device.get( ), 16);

	this->hook( );
	this->enable( );

	return 1;
}

void reset::Callback(D3DPRESENT_PARAMETERS*)
{
	gui::renderer::get_shared( )->reset(this->Target_instance( ));
}
