#pragma once

#include "cheat/hooks/base.h"

namespace cheat::csgo
{
	class IBaseClientDLL;
	enum ClientFrameStage_t;
}

namespace cheat::hooks::client
{
	struct frame_stage_notify final : hook_instance_shared<frame_stage_notify,__COUNTER__, void(csgo::IBaseClientDLL::*)(csgo::ClientFrameStage_t)>
	{
		frame_stage_notify( );

	protected:
		load_result load_impl( ) noexcept override;
		nstd::address get_target_method_impl( ) const override;
		void callback(csgo::ClientFrameStage_t stage) override;
	};
}
