module;

#include <cheat/hooks/instance.h>

#include <imgui_impl_dx9.h>
#include <d3d9.h>

module cheat.hooks.directx.reset;
import cheat.csgo.interfaces.Direct3DDevice9;
import cheat.gui;
import cheat.hooks.base;
import nstd.one_instance;
import nstd.mem.address;

using namespace cheat;
using namespace gui;
using namespace hooks;

using reset_base = hooks::base<decltype(&IDirect3DDevice9::Reset)>;
struct reset_impl :reset_base
{
	reset_impl( )
	{
		const nstd::mem::basic_address vtable_holder = csgo::Direct3DDevice9::get_ptr( );
		this->set_target_method(vtable_holder.deref<1>( )[16]);
	}

	void callback(D3DPRESENT_PARAMETERS*)
	{
		ImGui_ImplDX9_InvalidateDeviceObjects( );
		effects::invalidate_objects( );
	}
};

CHEAT_HOOK_INSTANCE(directx, reset);


