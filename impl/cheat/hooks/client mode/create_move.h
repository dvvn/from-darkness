#pragma once
// ReSharper disable once CppUnusedIncludeDirective
#include <intrin.h>
#include "cheat/hooks/base.h"

namespace cheat::csgo
{
	class IClientMode;
	class CUserCmd;
}

namespace cheat::hooks::client_mode
{
	struct create_move_impl final : service<create_move_impl>
								  , dhooks::_Detect_hook_holder_t<__COUNTER__, bool(csgo::IClientMode::*)(float, csgo::CUserCmd*)>
	{
		create_move_impl( );

	protected:
		load_result load_impl( ) noexcept override;
		void* get_target_method( ) const override;
		void callback(float input_sample_time, csgo::CUserCmd* cmd) override;
	};

	CHEAT_SERVICE_SHARE(create_move);
}
