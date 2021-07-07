#include "reset.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/gui/renderer.h"

using namespace cheat;
using namespace hooks;
using namespace directx;
using namespace utl;

reset::reset( )
{
	this->Wait_for<gui::renderer>( );
}

auto reset::Load( ) -> void
{
	this->target_func_ = method_info::make_member_virtual(csgo_interfaces::get( ).d3d_device.get( ), 16);

	this->hook( );
	this->enable( );
}

auto reset::Callback(D3DPRESENT_PARAMETERS*) -> void
{
	gui::renderer::get( ).reset(this->Target_instance( ));
}

