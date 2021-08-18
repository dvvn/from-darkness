#pragma once

#include "cheat/core/service.h"

#include "cheat/sdk/IClientMode.hpp"

namespace cheat::hooks::client_mode
{
	class create_move final: public service<create_move>,
							 public decltype(_Detect_hook_holder(&csgo::ClientModeShared::CreateMove)),
							 service_skipped_on_gui_test
	{
	public :
		create_move( );

	protected:
		bool Do_load( ) override;
		void Callback(float input_sample_time, csgo::CUserCmd* cmd) override;
	};
}
