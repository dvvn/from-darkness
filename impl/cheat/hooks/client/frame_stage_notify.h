#pragma once

#include "cheat/hooks/base.h"

namespace cheat::csgo
{
	class IBaseClientDLL;
	enum ClientFrameStage_t;
}

namespace cheat::hooks::client
{
	struct frame_stage_notify_impl final : service<frame_stage_notify_impl>
										 , dhooks::_Detect_hook_holder_t<__COUNTER__, void(csgo::IBaseClientDLL::*)(csgo::ClientFrameStage_t)>
	{
		frame_stage_notify_impl( );

	protected:
		load_result load_impl( ) noexcept override;
		nstd::address get_target_method_impl( ) const override;
		void callback(csgo::ClientFrameStage_t stage) override;
	};

	CHEAT_SERVICE_SHARE(frame_stage_notify);
}
