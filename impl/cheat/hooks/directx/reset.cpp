module;

#include "cheat/hooks/base_includes.h"
#include "cheat/gui/effects.h"
#include <imgui_impl_dx9.h>
#include <d3d9.h>

module cheat.hooks.directx:reset;
import cheat.gui;
import cheat.csgo.interfaces;

using namespace cheat;
using namespace gui;
using namespace hooks::directx;

reset::reset( ) = default;

void reset::construct( ) noexcept
{
	this->deps( ).add<csgo_interfaces >( );
	this->deps( ).add<gui::context>( );
}

bool reset::load( ) noexcept
{
	this->set_target_method(this->deps( ).get<csgo_interfaces>( ).d3d_device.vfunc(16).ptr( ));
	return hook_base::load( );
}

void reset::callback(D3DPRESENT_PARAMETERS*)
{
	ImGui_ImplDX9_InvalidateDeviceObjects( );
	effects::invalidate_objects( );
}