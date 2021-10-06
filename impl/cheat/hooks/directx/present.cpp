#include "present.h"
#include "reset.h"

#include "cheat/core/console.h"
#include "cheat/core/services loader.h"
#include "cheat/core/csgo interfaces.h"
#include "cheat/gui/menu.h"
#include "cheat/netvars/config.h"

#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

#include <d3d9.h>

using namespace cheat;
using namespace hooks;
using namespace directx;

present::present()
{
	this->wait_for_service<gui::menu>( );
}

CHEAT_HOOK_PROXY_INIT_FN(present, TRUE)
CHEAT_HOOK_PROXY_TARGET_FN(present, &csgo_interfaces::d3d_device, 17);

void present::callback(THIS_ CONST RECT*, CONST RECT*, HWND, CONST RGNDATA*)
{
	const auto d3d_device = this->object_instance;

#ifdef IMGUI_HAS_DOCK
	runtime_assert(ImGui::GetIO( ).ConfigFlags & ImGuiConfigFlags_DockingEnable, "docking and manual window title renderer are incompatible!");
#endif

	ImGui_ImplDX9_NewFrame( );   //todo: erase. it only calls CreateDeviceObjects, what can be done after reset and init
	ImGui_ImplWin32_NewFrame( ); //todo: call it from input (it only update mouse and keys). (if do it move timers outside)
	ImGui::NewFrame( );
	{
		const auto menu = gui::menu::get_ptr( );
#if CHEAT_GUI_HAS_DEMO_WINDOW && !defined(IMGUI_DISABLE_DEMO_WINDOWS)
#ifndef CHEAT_GUI_TEST
		if (menu->visible( ))
#endif
		ImGui::ShowDemoWindow( );
#endif
		menu->render( );
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

CHEAT_REGISTER_SERVICE(present);
