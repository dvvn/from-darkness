module;

#include "cheat/hooks/base_includes.h"
#include "cheat/gui/effects.h"

#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

#include <d3d9.h>

module cheat.hooks.directx:present;
import cheat.gui.menu;
import cheat.csgo.interfaces;

using namespace cheat;
using namespace gui;
using namespace hooks::directx;

present::present( )
{
	this->add_dependency(gui::menu::get( ));
}

void* present::get_target_method( ) const
{
	return csgo_interfaces::get( )->d3d_device.vfunc(17).ptr( );
}

void present::callback(THIS_ CONST RECT*, CONST RECT*, HWND, CONST RGNDATA*)
{
	const auto d3d_device = this->get_object_instance( );

#ifdef IMGUI_HAS_DOCK
	runtime_assert(ImGui::GetIO( ).ConfigFlags & ImGuiConfigFlags_DockingEnable, "docking and manual window title renderer are incompatible!");
#endif

	ImGui_ImplDX9_NewFrame( );   //todo: erase. it only calls CreateDeviceObjects, what can be done after reset and init
	ImGui_ImplWin32_NewFrame( ); //todo: call it from input (if do it move timers outside)

	const auto& io = ImGui::GetIO( );
	// Avoid rendering when minimized
	if (io.DisplaySize.x <= 0.0f || io.DisplaySize.y <= 0.0f)
		return;

	ImGui::NewFrame( );
	effects::new_frame( );
	{
		const auto& menu = menu::get( );
		[[maybe_unused]]
		const auto render_result = menu->render( );
#if !defined(IMGUI_DISABLE_DEMO_WINDOWS)
#ifndef CHEAT_GUI_TEST
		if (render_result)
#endif
			ImGui::ShowDemoWindow( );
#endif
	}
	ImGui::EndFrame( );

	[[maybe_unused]] const auto begin = d3d_device->BeginScene( );
	runtime_assert(SUCCEEDED(begin));
	{
		ImGui::Render( );
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData( ));
	}
	[[maybe_unused]] const auto end = d3d_device->EndScene( );
	runtime_assert(SUCCEEDED(end));
}

CHEAT_SERVICE_REGISTER(present);