#pragma once

#include "cheat/core/service.h"

#include "cheat/sdk/ISurface.hpp"

namespace cheat::hooks::vgui_surface
{
	class lock_cursor final: public service<lock_cursor>,
							 public decltype(_Detect_hook_holder(&csgo::ISurface::LockCursor)),
							 service_skipped_on_gui_test
	{
	public :
		lock_cursor( );

	protected:
		bool Do_load( ) override;
		void Callback( ) override;
	};
}
