#pragma once

#include "cheat/core/service.h"
#include "cheat/hooks/base.h"

namespace cheat::csgo
{
	class IClientMode;
	class CUserCmd;
}

namespace cheat::hooks::client_mode
{
	class create_move final: public base<create_move, bool(csgo::IClientMode::*)(float, csgo::CUserCmd*)>
						   , service_sometimes_skipped
	{
	public:
		create_move( );

	protected:
		nstd::address get_target_method_impl( ) const override;
		void          callback(float input_sample_time, csgo::CUserCmd* cmd) override;
	};
}
