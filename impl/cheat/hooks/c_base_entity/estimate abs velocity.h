#pragma once

#include "cheat/hooks/base.h"

namespace cheat::csgo
{
	class C_BaseEntity;
	class Vector;
}

namespace cheat::hooks::c_base_entity
{
	class estimate_abs_velocity final: public service_hook_proxy<estimate_abs_velocity, void(csgo::C_BaseEntity::*)(csgo::Vector&)>
			{
	public:
		estimate_abs_velocity( );

	protected:
		nstd::address get_target_method_impl( ) const override;
		void          callback(csgo::Vector& vel) override;
		load_result load_impl() noexcept override;
			};
}
