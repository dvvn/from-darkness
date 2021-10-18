#pragma once

#include "cheat/core/service.h"
#include "cheat/hooks/base.h"

namespace cheat::csgo
{
	class ISurface;
}

namespace cheat::hooks::vgui_surface
{
	CHEAT_SETUP_HOOK_PROXY(lock_cursor, void(csgo::ISurface::*)())
	{
		lock_cursor( );

	protected:
		load_result load_impl( ) noexcept override;
		nstd::address get_target_method_impl( ) const override;
		void callback( ) override;
	};
}
