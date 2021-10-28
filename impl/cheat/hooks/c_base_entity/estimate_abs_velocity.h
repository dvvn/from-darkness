#pragma once

#include "cheat/hooks/base.h"

namespace cheat::csgo
{
	class C_BaseEntity;
	class Vector;
}

namespace cheat::hooks::c_base_entity
{
	struct estimate_abs_velocity_impl final : service<estimate_abs_velocity_impl>
											, dhooks::_Detect_hook_holder_t<__COUNTER__, void(csgo::C_BaseEntity::*)(csgo::Vector&)>
	{
		estimate_abs_velocity_impl( );

	protected:
		nstd::address get_target_method_impl( ) const override;
		void callback(csgo::Vector& vel) override;
		load_result load_impl( ) noexcept override;
	};

	CHEAT_SERVICE_SHARE(estimate_abs_velocity);
}
