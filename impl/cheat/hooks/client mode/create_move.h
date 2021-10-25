#pragma once

#include "cheat/hooks/base.h"

namespace cheat::csgo
{
	class IClientMode;
	class CUserCmd;
}

namespace cheat::hooks::client_mode
{
	struct create_move final : hook_instance_shared<create_move,__COUNTER__, bool(csgo::IClientMode::*)(float, csgo::CUserCmd*)>
	{
		create_move( );

	protected:
		load_result load_impl( ) noexcept override;
		nstd::address get_target_method_impl( ) const override;
		void callback(float input_sample_time, csgo::CUserCmd* cmd) override;
	};
}
