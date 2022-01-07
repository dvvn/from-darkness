#if 0
#pragma once

#include "cheat/hooks/base.h"

namespace cheat::csgo
{
	// ReSharper disable once CppInconsistentNaming
	class C_BaseEntity;
}

namespace cheat::hooks::c_base_entity
{
	struct should_interpolate final: hook_instance_shared<should_interpolate,__COUNTER__,bool(csgo::C_BaseEntity::*)( )>
	{
		should_interpolate( );

	protected:
		nstd::address get_target_method_impl( ) const override;
		void callback( ) override;
		load_result load_impl( ) noexcept override;
	};
}
#endif