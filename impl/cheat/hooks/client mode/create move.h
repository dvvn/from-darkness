#pragma once

#include "cheat/core/service.h"

#include "cheat/sdk/IClientMode.hpp"

namespace cheat::hooks::client_mode
{
	class create_move final: public service<create_move>
						   , public decltype(_Detect_hook_holder(&csgo::ClientModeShared::CreateMove))
						   , service_hook_helper
#ifdef CHEAT_GUI_TEST
						   , service_always_skipped
#endif
	{
	protected:
		utl::address get_target_method_impl( ) const override;
		void         callback(float input_sample_time, csgo::CUserCmd* cmd) override;
	};
}
