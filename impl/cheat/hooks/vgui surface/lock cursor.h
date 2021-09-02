#pragma once

#include "cheat/core/service.h"
#include "cheat/hooks/base.h"

namespace cheat::csgo
{
	class ISurface;
}

namespace cheat::hooks::vgui_surface
{
	class lock_cursor final: public hook_base<lock_cursor, void(csgo::ISurface::*)( )>
						   , service_sometimes_skipped

	{
	public:
		lock_cursor( );

	protected:
		nstd::address get_target_method_impl( ) const override;
		void          callback( ) override;
	};
}
