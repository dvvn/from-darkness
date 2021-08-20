#pragma once

#include "cheat/core/service.h"
#include "cheat/sdk/entity/C_BaseEntity.h"

namespace cheat::hooks::c_base_entity
{
	class estimate_abs_velocity final: public service<estimate_abs_velocity>
									 , public decltype(_Detect_hook_holder(&csgo::C_BaseEntity::EstimateAbsVelocity))
									 , service_hook_helper
#ifdef CHEAT_GUI_TEST
									 , service_always_skipped
#endif
	{
	protected:
		utl::address get_target_method_impl( ) const override;
		void         callback(csgo::Vector& vel) override;
	};
}
