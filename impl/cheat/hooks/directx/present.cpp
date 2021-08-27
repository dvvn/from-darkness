#include "present.h"
#include "reset.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/gui/menu.h"

using namespace cheat;
using namespace hooks;
using namespace directx;

present::present( )
{
	this->add_service<gui::menu>( );
}

nstd::address present::get_target_method_impl( ) const
{
	return dhooks::_Pointer_to_virtual_class_table(csgo_interfaces::get_ptr( )->d3d_device.get( ))[17];
}

void present::callback(THIS_ CONST RECT*, CONST RECT*, HWND, CONST RGNDATA*)
{
	const auto d3d_device = this->object_instance;

	ImGui_ImplDX9_NewFrame( );   //todo: remove. it only calls CreateDeviceObjects, what can be done after reset and init
	ImGui_ImplWin32_NewFrame( ); //todo: call it from input (it only update mouse and keys). (if do it move timers outside)
	ImGui::NewFrame( );
	{
		const auto menu = gui::menu::get_ptr( );
#if CHEAT_GUI_HAS_DEMO_WINDOW
#ifndef CHEAT_GUI_TEST
		if (menu->active( ))
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
