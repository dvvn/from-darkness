#pragma once

#include "cheat/core/service.h"

#include "cheat/sdk/ISurface.hpp"

namespace cheat::hooks::vgui_surface
{
	class lock_cursor final: public service_shared<lock_cursor, service_mode::async>,
									public decltype(detect_hook_holder(&csgo::ISurface::LockCursor))
	{
	public :
		lock_cursor( );

	protected:
		void Load( ) override;
		void Callback() override;
	};
}
