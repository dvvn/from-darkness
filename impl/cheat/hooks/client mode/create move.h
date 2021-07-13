#pragma once

#include "cheat/core/service.h"

#include "cheat/sdk/IClientMode.hpp"

namespace cheat::hooks::client_mode
{
	class create_move final: public service_shared<create_move, service_mode::async>,
							 public decltype(detect_hook_holder(&csgo::ClientModeShared::CreateMove)),
							 public service_top_level_only_tag
	{
	public :
		create_move( );

	protected:
		void Load( ) override;
		void Callback(float input_sample_time, csgo::CUserCmd* cmd) override;
	};
}
