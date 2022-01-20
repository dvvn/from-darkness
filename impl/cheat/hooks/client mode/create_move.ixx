module;

#include "cheat/hooks/base_includes.h"

export module cheat.hooks.client_mode:create_move;
import cheat.hooks.base;
import cheat.csgo.interfaces;

export namespace cheat::hooks::client_mode
{
	struct create_move final : hook_base<create_move, bool(csgo::IClientMode::*)(float, csgo::CUserCmd*)>
	{
		create_move( );

	protected:
		void construct( ) noexcept override;
		void* get_target_method( ) const override;
		void callback(float input_sample_time, csgo::CUserCmd* cmd) override;
	};
}
