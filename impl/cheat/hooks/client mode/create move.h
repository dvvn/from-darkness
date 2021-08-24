#pragma once

#include "cheat/core/service.h"

#include "cheat/sdk/IClientMode.hpp"

namespace cheat::hooks::client_mode
{
	class create_move final: public service<create_move>
						   , public dhooks::_Detect_hook_holder_t<decltype(&csgo::ClientModeShared::CreateMove)>
						   , service_hook_helper
#if defined(CHEAT_GUI_TEST) || defined(CHEAT_NETVARS_UPDATING)
							 , service_always_skipped
#endif
	{
	protected:
		nstd::address get_target_method_impl( ) const override;
		void          callback(float input_sample_time, csgo::CUserCmd* cmd) override;
	};
}
