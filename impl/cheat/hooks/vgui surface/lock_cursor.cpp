module;

#include <cheat/hooks/instance.h>

module cheat.hooks.vgui_surface.lock_cursor;
import cheat.gui;
import cheat.csgo.interfaces.VguiSurface;
import cheat.hooks.base;
import nstd.mem.address;

using namespace cheat;
using namespace csgo;
using namespace hooks;

CHEAT_HOOK_INSTANCE(vgui_surface, lock_cursor);

static void* target( ) noexcept
{
	const nstd::mem::basic_address<void> vtable_holder = ISurface::get_ptr( );
	return vtable_holder.deref<1>( )[67];
}

struct replace
{
	void fn( ) noexcept
	{
		auto inst = reinterpret_cast<ISurface*>(this);
		if (!inst->IsCursorVisible( ) && gui::menu::visible( ))
			inst->UnlockCursor( );
		else
			CHEAT_HOOK_CALL_ORIGINAL_MEMBER( );
	}
};

CHEAT_HOOK_INIT(vgui_surface, lock_cursor);
