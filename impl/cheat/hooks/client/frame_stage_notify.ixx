module;

#include "cheat/hooks/base_includes.h"

export module cheat.hooks.client:frame_stage_notify;
import cheat.hooks.base;
import cheat.csgo.interfaces;

namespace cheat::hooks::client
{
	struct frame_stage_notify final : hook_base<frame_stage_notify, void(csgo::IBaseClientDLL::*)(csgo::ClientFrameStage_t)>
	{
		frame_stage_notify( );

	protected:
		void* get_target_method( ) const override;
		void callback(csgo::ClientFrameStage_t stage) override;
	};
}