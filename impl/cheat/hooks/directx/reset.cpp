module;

#include "cheat/hooks/base_includes.h"
#include "cheat/gui/effects.h"
#include <imgui_impl_dx9.h>
#include <d3d9.h>

module cheat.hooks.directx:reset;
import cheat.gui.context;
import cheat.csgo.interfaces;

using namespace cheat;
using namespace gui;
using namespace hooks::directx;

reset::reset( ) = default;

void reset::load_async( ) noexcept
{
	this->deps( ).add<gui::context>( );
}

void* reset::get_target_method( ) const
{
	return services_loader::get( ).deps( ).get<csgo_interfaces>( ).d3d_device.vfunc(16).ptr( );
}

void reset::callback(D3DPRESENT_PARAMETERS*)
{
	ImGui_ImplDX9_InvalidateDeviceObjects( );
	effects::invalidate_objects( );
}

CHEAT_SERVICE_REGISTER(reset);
