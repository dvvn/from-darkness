module;

export module cheat.hooks.client:frame_stage_notify;
import cheat.csgo.interfaces.BaseClient;
import dhooks;

namespace cheat::hooks::client
{
	export class frame_stage_notify final :public dhooks::select_hook_holder< void(csgo::IBaseClientDLL::*)(csgo::ClientFrameStage_t)>
	{
	public:
		frame_stage_notify( );

	protected:
		void callback(csgo::ClientFrameStage_t stage) override;
	};
}
