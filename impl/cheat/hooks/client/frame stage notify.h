#pragma once

#include "cheat/core/service.h"

#include "cheat/sdk/IBaseClientDll.hpp"

namespace cheat::hooks::client
{
	class frame_stage_notify final: public service<frame_stage_notify>,
									public decltype(_Detect_hook_holder(&csgo::IBaseClientDLL::FrameStageNotify)),
	service_skipped_on_gui_test
	{
	public:
		frame_stage_notify( );

	protected:
		bool Do_load( ) override;
		void Callback(csgo::ClientFrameStage_t stage) override;
	};
}
