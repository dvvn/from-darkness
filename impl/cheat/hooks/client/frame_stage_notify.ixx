module;

#include "cheat/hooks/base_includes.h"

export module cheat.hooks.client:frame_stage_notify;
import cheat.hooks.base;
import cheat.csgo.interfaces.BaseClient;

namespace cheat::hooks::client
{
	export class frame_stage_notify final :public hook_base<frame_stage_notify, void(csgo::IBaseClientDLL::*)(csgo::ClientFrameStage_t)>
	{
	public:
		frame_stage_notify( );

	protected:
		void construct( ) noexcept override;
		void callback(csgo::ClientFrameStage_t stage) override;
	};
}
