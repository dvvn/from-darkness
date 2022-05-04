module;

//#include <nstd/runtime_assert.h>

//#include <imgui_impl_dx9.h>
//#include <imgui_impl_win32.h>

#include <d3d9.h>

#include <string_view>

module cheat.hooks.directx.present;
import cheat.gui.context;
import cheat.hooks.hook;

using namespace cheat;
using namespace hooks;
using namespace directx;

struct present_impl final : present, hook, hook_instance_member<present_impl>
{
	present_impl( )
	{
		entry_type entry;
		entry.set_target_method({&nstd::instance_of<IDirect3DDevice9*>, 17,&IDirect3DDevice9::Present});
		entry.set_replace_method(&present_impl::callback);

		this->init(std::move(entry));
	}

	HRESULT WINAPI callback(THIS_ CONST RECT* source_rect, CONST RECT* desc_rect, HWND dest_window_override, CONST RGNDATA* dirty_region) const noexcept
	{
#if 0
		using namespace gui;

		//#ifdef IMGUI_HAS_DOCK
		//		runtime_assert(context::get( ).IO.ConfigFlags & ImGuiConfigFlags_DockingEnable, "docking and manual window title renderer are incompatible!");
		//#endif

		ImGui_ImplDX9_NewFrame( );   //todo: erase. it only calls CreateDeviceObjects, what can be done after reset and init
		ImGui_ImplWin32_NewFrame( ); //todo: call it from input (if do it move timers outside)

		const auto& io = context::get( ).IO;
		// Avoid rendering when minimized
		if(io.DisplaySize.x <= 0.0f || io.DisplaySize.y <= 0.0f)
			return CHEAT_HOOK_CALL_ORIGINAL_MEMBER(source_rect, desc_rect, dest_window_override, dirty_region);

		ImGui::NewFrame( );
		effects::new_frame( );
		{
			[[maybe_unused]]
			const auto render_result = menu::render( );
#ifndef IMGUI_DISABLE_DEMO_WINDOWS
			if(render_result)
				ImGui::ShowDemoWindow( );
#endif
		}
		ImGui::EndFrame( );

		const auto d3d_device = reinterpret_cast<IDirect3DDevice9*>(this);

		[[maybe_unused]] const auto begin = d3d_device->BeginScene( );
		runtime_assert(SUCCEEDED(begin));
		{
			ImGui::Render( );
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData( ));
		}
		[[maybe_unused]] const auto end = d3d_device->EndScene( );
		runtime_assert(SUCCEEDED(end));

#endif
		nstd::instance_of<gui::context>->render( );
		return call_original(source_rect, desc_rect, dest_window_override, dirty_region);
	}
};

std::string_view present::class_name( ) const noexcept
{
	return "hooks::directx";
}

std::string_view present::function_name( ) const noexcept
{
	return "present";
}

template<>
template<>
nstd::one_instance_getter<present*>::one_instance_getter(const std::in_place_index_t<0>)
	:item_(present_impl::get_ptr( ))
{
}
