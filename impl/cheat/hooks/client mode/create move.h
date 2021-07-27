#pragma once

#include "cheat/core/service.h"

#include "cheat/sdk/IClientMode.hpp"

namespace cheat::hooks::client_mode
{
	class create_move final: public service<create_move>,
							 public decltype(detect_hook_holder(&csgo::ClientModeShared::CreateMove))
	{
	public :
		create_move( );

	protected:
		bool Do_load( ) override;
		void Callback(float input_sample_time, csgo::CUserCmd* cmd) override;
	};
}
