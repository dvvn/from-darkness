#pragma once

#include "cheat/core/service.h"
#include "cheat/hooks/base.h"

namespace cheat::csgo
{
	class ISurface;
}

namespace cheat::hooks::vgui_surface
{
	struct lock_cursor_impl final : service<lock_cursor_impl>, dhooks::_Detect_hook_holder_t<__COUNTER__, void(csgo::ISurface::*)( )>
	{
		lock_cursor_impl( );

	protected:
		load_result load_impl( ) noexcept override;
		nstd::address get_target_method_impl( ) const override;
		void callback( ) override;
	};

	CHEAT_SERVICE_SHARE(lock_cursor);
}
