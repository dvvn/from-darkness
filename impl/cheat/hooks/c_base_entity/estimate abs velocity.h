#pragma once

#include "cheat/core/service.h"
#include "cheat/sdk/entity/C_BaseEntity.h"

namespace cheat::hooks::c_base_entity
{
	class estimate_abs_velocity final: public service<estimate_abs_velocity>,
									   public decltype(_Detect_hook_holder(&csgo::C_BaseEntity::EstimateAbsVelocity))
	{
	public :
		estimate_abs_velocity( );

	protected:
		bool Do_load( ) override;
		void Callback(csgo::Vector& vel) override;
	};
}
