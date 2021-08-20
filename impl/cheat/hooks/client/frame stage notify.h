#pragma once

#include "cheat/core/service.h"

#include "cheat/sdk/IBaseClientDll.hpp"

namespace cheat::hooks::client
{
	class frame_stage_notify final: public service<frame_stage_notify>
								  , public decltype(_Detect_hook_holder(&csgo::IBaseClientDLL::FrameStageNotify))
								  , service_hook_helper
#ifdef CHEAT_GUI_TEST
								  , service_always_skipped
#endif
	{
	protected:
		utl::address get_target_method_impl( ) const override;
		void         callback(csgo::ClientFrameStage_t stage) override;
	};
}
