#pragma once

#include "cheat/hooks/base.h"

namespace cheat::csgo
{
	class IBaseClientDLL;
	enum ClientFrameStage_t;
}

namespace cheat::hooks::client
{
	CHEAT_SETUP_HOOK_PROXY(frame_stage_notify, void(csgo::IBaseClientDLL::*)(csgo::ClientFrameStage_t))
	{
		frame_stage_notify( );

	protected:
		load_result load_impl( ) noexcept override;
		nstd::address get_target_method_impl( ) const override;
		void callback(csgo::ClientFrameStage_t stage) override;
	};
}
