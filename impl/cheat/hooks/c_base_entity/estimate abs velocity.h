#pragma once

#include "cheat/core/service.h"
#include "cheat/sdk/entity/C_BaseEntity.h"

namespace cheat::hooks::c_base_entity
{
	class estimate_abs_velocity final: public service_shared<estimate_abs_velocity, service_mode::async>,
									   public decltype(detect_hook_holder(&csgo::C_BaseEntity::EstimateAbsVelocity))
	{
	public :
		estimate_abs_velocity( );

	protected:
		void Load( ) override;
		void Callback(utl::Vector& vel) override;
	};
}
