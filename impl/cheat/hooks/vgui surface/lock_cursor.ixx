module;

#include "cheat/hooks/base_includes.h"

export module cheat.hooks.vgui_surface:lock_cursor;
import cheat.hooks.base;
import cheat.csgo.interfaces.VguiSurface;

namespace cheat::hooks::vgui_surface
{
	export class lock_cursor final :public hook_base<lock_cursor, void(csgo::ISurface::*)()>
	{
	public:
		lock_cursor( );

	protected:
		void construct( ) noexcept override;
		void callback( ) override;
	};
}
