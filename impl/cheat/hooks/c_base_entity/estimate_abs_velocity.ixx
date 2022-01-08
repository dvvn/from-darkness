module;

#include "cheat/hooks/base_includes.h"

export module cheat.hooks.c_base_entity:estimate_abs_velocity;
import cheat.hooks.base;
import cheat.csgo.interfaces;

//namespace cheat::csgo
//{
//	class C_BaseEntity;
//	class Vector;
//}

namespace cheat::hooks::c_base_entity
{
	struct estimate_abs_velocity final : hook_base<estimate_abs_velocity, void(csgo::C_BaseEntity::*)(csgo::Vector&)>
	{
		estimate_abs_velocity( );

	protected:
		void load_async( ) noexcept override;
		void* get_target_method( ) const override;
		void callback(csgo::Vector& vel) override;
	};

}
