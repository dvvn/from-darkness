module;

#include "cheat/hooks/base_includes.h"

export module cheat.hooks.client_mode:create_move;
import cheat.hooks.base;
import cheat.csgo.interfaces;

namespace cheat::hooks::client_mode
{
	export struct create_move final :public hook_base<create_move, decltype(&csgo::IClientMode::CreateMove)>
	{
	public:
		create_move( );

	protected:
		void construct( ) noexcept override;
		bool load( ) noexcept override;
		void callback(float input_sample_time, csgo::CUserCmd* cmd) override;
	};
}
