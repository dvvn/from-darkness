#pragma once

#include "cheat/core/service.h"
#include "cheat/sdk/entity/C_BaseEntity.h"

namespace cheat::hooks::c_base_entity
{
	class should_interpolate final: public service<should_interpolate>,
									public decltype(_Detect_hook_holder(&csgo::C_BaseEntity::ShouldInterpolate)),
									service_skipped_always
	{
	public:
		should_interpolate( );

	protected:
		bool Do_load( ) override;
		void Callback( ) override;
	};
}
