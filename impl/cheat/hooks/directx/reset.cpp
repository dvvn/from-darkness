module;

//#include <imgui_impl_dx9.h>
#include <d3d9.h>

#include <string_view>

module cheat.hooks.directx.reset;
import cheat.hooks.hook;
import cheat.gui.render_interface;

using namespace cheat;
using namespace hooks;
using namespace directx;

struct  reset_impl final : reset, hook, hook_instance_member<reset_impl>
{
	reset_impl( )
	{
		entry_type entry;
		entry.set_target_method({&nstd::instance_of<IDirect3DDevice9*>, 16, &IDirect3DDevice9::Reset});
		entry.set_replace_method(&reset_impl::callback);

		this->init(std::move(entry));
	}

	[[noreturn]]
	 void WINAPI callback(D3DPRESENT_PARAMETERS* params) const noexcept
	{
		//ImGui_ImplDX9_InvalidateDeviceObjects( );
		//gui::effects::invalidate_objects( );
		nstd::instance_of<gui::render_interface>->ReleaseTextures( );
		call_original(params);
	}
};

std::string_view reset::class_name( ) const noexcept
{
	return "hooks::directx";
}

std::string_view reset::function_name( ) const noexcept
{
	return "reset";
}

template<>
template<>
nstd::one_instance_getter<reset*>::one_instance_getter(const std::in_place_index_t<0>)
	:item_(reset_impl::get_ptr( ))
{
}