#include "renderer.h"

#include "user input.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/gui/menu/menu.h"
#include "cheat/hooks/input/wndproc.h"

using namespace cheat;
using namespace gui;
using namespace utl;

renderer::renderer( )
{
	this->Wait_for<menu_obj>( );
	this->Wait_for<hooks::input::wndproc>( );
}

renderer::~renderer( )
{
	using namespace utl::hooks;
	using namespace mem;

	auto test = method_info::make_member_virtual<IDirect3DDevice9*>(csgo_interfaces::get( ).d3d_device, 1);
	if (!test.update( ))
		return;
	if (!memory_block(test.get( )).executable( )) //if not - game closed
		return;
	ImGui_ImplDX9_Shutdown( );
}

void renderer::Load( )
{
	IDirect3DDevice9* d3d = csgo_interfaces::get( ).d3d_device;
	ImGui_ImplDX9_Init(d3d);
	//ImGui_ImplDX9_CreateDeviceObjects( ); d3d9 multithread error
}

void renderer::present(IDirect3DDevice9* d3d_device)
{
	const auto update_imgui_impl = []
	{
		ImGui_ImplDX9_NewFrame( );   //todo: remove. it only calls CreateDeviceObjects, what can be done after reset and init
		ImGui_ImplWin32_NewFrame( ); //todo: call it from input (it only update mouse and keys). if do it move timers outside
	};

	if (skip_first_tick__)
	{
		skip_first_tick__ = false;
		//update timers & shits
		return update_imgui_impl( );
	}

	auto& menu = menu_obj::get( );
	auto  bg_alpha = 1.f;
	if (menu.animate( ))
	{
		bg_alpha = menu.get_fade( );
	}
	else if (!menu.visible( ))
	{
		skip_first_tick__ = true;
		return;
	}

	update_imgui_impl( );
	//todo: if no keys pressed, mouse stay, no animations, skip it and go to render (save cpu a little)
	//todo: if imgui have nothing to draw return
	ImGui::NewFrame( );
	{
		menu.render(bg_alpha);
	}
	ImGui::EndFrame( );

	const auto begin = d3d_device->BeginScene( );
	BOOST_ASSERT(SUCCEEDED(begin));
	{
		ImGui::Render( );
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData( ));
	}
	const auto end = d3d_device->EndScene( );
	BOOST_ASSERT(SUCCEEDED(end));
}

void renderer::reset([[maybe_unused]] IDirect3DDevice9* d3d_device)
{
	(void)this;
	ImGui_ImplDX9_InvalidateDeviceObjects( );
}
