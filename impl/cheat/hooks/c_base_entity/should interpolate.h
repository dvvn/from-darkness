#pragma once

#include "cheat/core/service.h"
#include "cheat/sdk/entity/C_BaseEntity.h"

namespace cheat::hooks::c_base_entity
{
	class should_interpolate final: public service<should_interpolate>
								  , public decltype(_Detect_hook_holder(&csgo::C_BaseEntity::ShouldInterpolate))
								  , service_hook_helper
								  , service_always_skipped
	{
	protected:
		utl::address get_target_method_impl( ) const override;
		void         callback( ) override;
	};
}
