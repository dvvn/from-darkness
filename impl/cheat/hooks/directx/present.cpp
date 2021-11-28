#include "present.h"

#include "cheat/gui/menu.h"
#include "cheat/gui/effects.h"

#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

#include <d3d9.h>

using namespace cheat;
using namespace gui;
using namespace hooks::directx;

void present_impl::callback(THIS_ CONST RECT*, CONST RECT*, HWND, CONST RGNDATA*)
{
	const auto d3d_device = this->get_object_instance( );

#ifdef IMGUI_HAS_DOCK
	runtime_assert(ImGui::GetIO( ).ConfigFlags & ImGuiConfigFlags_DockingEnable, "docking and manual window title renderer are incompatible!");
#endif

	ImGui_ImplDX9_NewFrame( );   //todo: erase. it only calls CreateDeviceObjects, what can be done after reset and init
	ImGui_ImplWin32_NewFrame( ); //todo: call it from input (it only update mouse and keys). (if do it move timers outside)
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
