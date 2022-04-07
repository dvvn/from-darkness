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
using namespace hooks;

CHEAT_HOOK_INSTANCE(directx, reset);

static void* target( ) noexcept
{
	const nstd::mem::basic_address vtable_holder = csgo::Direct3DDevice9::get_ptr( );
	return vtable_holder.deref<1>( )[16];
}

struct replace
{
	void WINAPI fn(D3DPRESENT_PARAMETERS* params) noexcept
	{
		ImGui_ImplDX9_InvalidateDeviceObjects( );
		gui::effects::invalidate_objects( );
		CHEAT_HOOK_CALL_ORIGINAL_MEMBER(params);
	}
};

CHEAT_HOOK_INIT(directx, reset);
