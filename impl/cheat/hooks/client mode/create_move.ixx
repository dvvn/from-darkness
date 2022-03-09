module;


export module cheat.hooks.client_mode:create_move;
import cheat.csgo.interfaces.ClientMode;
import dhooks;

namespace cheat::hooks::client_mode
{
	export struct create_move final :public dhooks::select_hook_holder<decltype(&csgo::IClientMode::CreateMove)>
	{
	public:
		create_move( );

	protected:
		void callback(float input_sample_time, csgo::CUserCmd* cmd) override;
	};
}
