module;

#include <string_view>

module cheat.hooks.vgui_surface.lock_cursor;
import cheat.hooks.hook;
//import cheat.gui;
import cheat.csgo.interfaces.VguiSurface;

using namespace cheat;
using namespace csgo;
using namespace hooks;
using namespace vgui_surface;

#if 0
CHEAT_HOOK_INSTANCE(vgui_surface, lock_cursor);

static void* target( ) noexcept
{
	const nstd::mem::basic_address<void> vtable_holder = &nstd::instance_of<ISurface*>;
	return vtable_holder.deref<1>( )[67];
}

struct replace
{
	void fn( ) noexcept
	{
		auto inst = reinterpret_cast<ISurface*>(this);
		if(!inst->IsCursorVisible( ) && gui::menu::visible( ))
			inst->UnlockCursor( );
		else
			CHEAT_HOOK_CALL_ORIGINAL_MEMBER( );
	}
};

CHEAT_HOOK_INIT(vgui_surface, lock_cursor);
#endif

struct lock_cursor_impl final : lock_cursor, hook, hook_instance_member<lock_cursor_impl>
{
	lock_cursor_impl( )
	{
		entry_type entry;
		entry.set_target_method({&nstd::instance_of<ISurface*>, 67});
		entry.set_replace_method(&lock_cursor_impl::callback);

		this->init(std::move(entry));
	}

	void callback( ) const noexcept
	{
		/*auto thisptr = reinterpret_cast<ISurface*>(this);
		if(!thisptr->IsCursorVisible( ) && gui::menu::visible( ))
			thisptr->UnlockCursor( );
		else
			CHEAT_HOOK_CALL_ORIGINAL_MEMBER( );*/

		call_original( );
	}


};

std::string_view lock_cursor::class_name( ) const noexcept
{
	return "hooks::vgui_surface";
}

std::string_view lock_cursor::function_name( ) const noexcept
{
	return "lock_cursor";
}

template<>
template<>
nstd::one_instance_getter<lock_cursor*>::one_instance_getter(const std::in_place_index_t<0>)
	:item_(lock_cursor_impl::get_ptr( ))
{
}