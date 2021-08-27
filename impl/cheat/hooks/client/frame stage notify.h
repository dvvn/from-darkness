#pragma once

#include "cheat/core/service.h"

#include "cheat/sdk/IBaseClientDll.hpp"

namespace cheat::hooks::client
{
	class frame_stage_notify final: public service<frame_stage_notify>
								  , public dhooks::_Detect_hook_holder_t<decltype(&csgo::IBaseClientDLL::FrameStageNotify)>
								  , service_hook_helper
#if defined(CHEAT_GUI_TEST) || defined(CHEAT_NETVARS_UPDATING)
								  , service_always_skipped
#endif
	{
	public:
		frame_stage_notify( );

	protected:
		nstd::address get_target_method_impl( ) const override;
		void          callback(csgo::ClientFrameStage_t stage) override;
	};
}
