module;

#include "cheat/hooks/base_includes.h"

export module cheat.hooks.vgui_surface:lock_cursor;
import cheat.hooks.base;
import cheat.csgo.interfaces;

namespace cheat::hooks::vgui_surface
{
	export struct lock_cursor final : hook_base<lock_cursor, void(csgo::ISurface::*)()>
	{
		lock_cursor( );

	protected:
		void load_async( ) noexcept override;
		void* get_target_method( ) const override;
		void callback( ) override;
	};
}
