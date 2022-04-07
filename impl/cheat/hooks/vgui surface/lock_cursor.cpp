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

#if 0
using lock_cursor_base = hooks::base<void(ISurface::*)()>;
struct lock_cursor_impl : lock_cursor_base
{
	lock_cursor_impl( )
	{
		//this->set_target_method(this->deps( ).get<csgo_interfaces>( ).vgui_surface.vfunc(67));
		this->set_target_method(nstd::mem::basic_address(ISurface::get_ptr( )).deref<1>( )[67]);
	}

	void callback( )
	{
		auto inst = this->get_object_instance( );
		if (!inst->IsCursorVisible( ) && gui::menu::visible( ))
		{
			this->store_return_value( );
			inst->UnlockCursor( );
		}
	}
};
#endif

CHEAT_HOOK_INSTANCE(vgui_surface, lock_cursor);

static void* target( ) noexcept
{
	return nstd::mem::basic_address(ISurface::get_ptr( )).deref<1>( )[67];
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
