#pragma once

#include "cheat/core/service.h"
#include "cheat/sdk/entity/C_BaseEntity.h"

namespace cheat::hooks::c_base_entity
{
	class estimate_abs_velocity final: public service<estimate_abs_velocity>
									 , public dhooks::_Detect_hook_holder_t<decltype(&csgo::C_BaseEntity::EstimateAbsVelocity)>
									 , service_hook_helper
#if defined(CHEAT_GUI_TEST) || defined(CHEAT_NETVARS_UPDATING)
									 , service_always_skipped
#endif
	{
	public:
		estimate_abs_velocity( );

	protected:
		nstd::address get_target_method_impl( ) const override;
		void          callback(csgo::Vector& vel) override;
	};
}
