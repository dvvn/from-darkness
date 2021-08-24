#pragma once

#include "cheat/core/service.h"

#include "cheat/sdk/ISurface.hpp"

namespace cheat::hooks::vgui_surface
{
	class lock_cursor final: public service<lock_cursor>
						   , public dhooks::_Detect_hook_holder_t<decltype(&csgo::ISurface::LockCursor)>
						   , service_hook_helper
#ifdef CHEAT_GUI_TEST
						   , service_always_skipped
#endif
	{
	protected:
		nstd::address get_target_method_impl( ) const override;
		void          callback( ) override;
	};
}
