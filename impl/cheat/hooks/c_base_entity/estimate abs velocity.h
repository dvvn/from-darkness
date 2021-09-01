#pragma once

#include "cheat/core/service.h"
#include "cheat/hooks/base.h"

namespace cheat::csgo
{
	class C_BaseEntity;
	class Vector;
}

namespace cheat::hooks::c_base_entity
{
	class estimate_abs_velocity final: public base<estimate_abs_velocity, void(csgo::C_BaseEntity::*)(csgo::Vector&)>
									 , service_sometimes_skipped

	{
	public:
		estimate_abs_velocity( );

	protected:
		nstd::address get_target_method_impl( ) const override;
		void          callback(csgo::Vector& vel) override;
	};
}
