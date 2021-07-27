#include "renderer.h"

#include "user input.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/gui/menu.h"
#include "cheat/hooks/input/wndproc.h"
#include "cheat/hooks/vgui surface/lock cursor.h"

using namespace cheat;
using namespace hooks;
using namespace gui;
using namespace utl;

renderer::renderer( )
{
}

renderer::~renderer( )
{
	invoke([]
	{
		__try
		{
			ImGui_ImplDX9_Shutdown( );
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}
	});
}

bool renderer::Do_load( )
{
	IDirect3DDevice9* d3d = csgo_interfaces::get_shared( )->d3d_device;
	ImGui_ImplDX9_Init(d3d);
	//ImGui_ImplDX9_CreateDeviceObjects( ); d3d9 multithread error

	return true;
}

void renderer::present(IDirect3DDevice9* d3d_device)
{
	(void)this;

	ImGui_ImplDX9_NewFrame( );   //todo: remove. it only calls CreateDeviceObjects, what can be done after reset and init
	ImGui_ImplWin32_NewFrame( ); //todo: call it from input (it only update mouse and keys). (if do it move timers outside)

	ImGui::NewFrame( );
	{
#if CHEAT_GUI_HAS_DEMO_WINDOW
#ifndef CHEAT_TEST_EXE
		if (menu::get_shared( )->active( ))
#endif
			ImGui::ShowDemoWindow( );
#endif
		menu::get_shared( )->render( );
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

void renderer::reset([[maybe_unused]] IDirect3DDevice9* d3d_device)
{
	(void)this;
	ImGui_ImplDX9_InvalidateDeviceObjects( );
}
