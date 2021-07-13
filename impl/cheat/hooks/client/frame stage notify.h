#pragma once

#include "cheat/core/service.h"

#include "cheat/sdk/IBaseClientDll.hpp"

namespace cheat::hooks::client
{
	class frame_stage_notify final: public service_shared<frame_stage_notify, service_mode::async>,
									public decltype(detect_hook_holder(&csgo::IBaseClientDLL::FrameStageNotify)),
									public service_top_level_only_tag
	{
	public :
		frame_stage_notify( );

	protected:
		void Load( ) override;
		void Callback(csgo::ClientFrameStage_t stage) override;
	};
}
