module;

#include <nstd/runtime_assert.h>

#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

#include <d3d9.h>

module cheat.hooks.directx:present;
import cheat.csgo.interfaces.Direct3DDevice9;
import cheat.gui;
import nstd.mem.address;

using namespace cheat;
using namespace gui;
using namespace hooks::directx;

present::present( )
{
	const nstd::mem::basic_address vtable_holder = csgo::Direct3DDevice9::get_ptr( );
	this->set_target_method(vtable_holder.deref<1>( )[17]);
}

void present::callback(THIS_ CONST RECT*, CONST RECT*, HWND, CONST RGNDATA*)
{
	const auto d3d_device = this->get_object_instance( );

#ifdef IMGUI_HAS_DOCK
	runtime_assert(context::get( ).IO.ConfigFlags & ImGuiConfigFlags_DockingEnable, "docking and manual window title renderer are incompatible!");
#endif

	ImGui_ImplDX9_NewFrame( );   //todo: erase. it only calls CreateDeviceObjects, what can be done after reset and init
	ImGui_ImplWin32_NewFrame( ); //todo: call it from input (if do it move timers outside)

	const auto& io = context::get( ).IO;
	// Avoid rendering when minimized
	if (io.DisplaySize.x <= 0.0f || io.DisplaySize.y <= 0.0f)
		return;

	ImGui::NewFrame( );
	effects::new_frame( );
	{
		[[maybe_unused]]
		const auto render_result = menu::render( );
#if !defined(IMGUI_DISABLE_DEMO_WINDOWS)
		if (render_result)
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
