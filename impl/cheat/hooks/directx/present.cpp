#include "present.h"
#include "reset.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/gui/menu.h"

using namespace cheat;
using namespace hooks;
using namespace directx;
using namespace utl;

present::present( )
{
}

bool present::Do_load( )
{
	target_func_ = method_info::make_member_virtual(csgo_interfaces::get_ptr( )->d3d_device.get( ), 17);

	this->hook( );
	this->enable( );

	return true;
}

void present::Callback(THIS_ CONST RECT*, CONST RECT*, HWND, CONST RGNDATA*)
{
	const auto d3d_device = this->Target_instance( );

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
	BOOST_ASSERT(SUCCEEDED(begin));
	{
		ImGui::Render( );
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData( ));
	}
	[[maybe_unused]] const auto end = d3d_device->EndScene( );
	BOOST_ASSERT(SUCCEEDED(end));
}
