module;

#include "cheat/hooks/base_includes.h"
#include <imgui_impl_dx9.h>

#include <d3d9.h>

module cheat.hooks.directx:reset;
import cheat.csgo.interfaces.Direct3DDevice9;
import cheat.gui;
import nstd.mem.address;

using namespace cheat;
using namespace gui;
using namespace hooks::directx;

reset::reset( )
{
	//this->set_target_method(this->deps( ).get<csgo_interfaces>( ).d3d_device.vfunc(16));
	const nstd::mem::basic_address vtable_holder = csgo::Direct3DDevice9::get_ptr( );
	this->set_target_method(vtable_holder.deref<1>( )[16]);
}

void reset::construct( ) noexcept
{

}

void reset::callback(D3DPRESENT_PARAMETERS*)
{
	ImGui_ImplDX9_InvalidateDeviceObjects( );
	effects::invalidate_objects( );
}