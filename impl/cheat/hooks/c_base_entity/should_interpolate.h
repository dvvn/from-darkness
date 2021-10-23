#pragma once

#include "cheat/hooks/base.h"

namespace cheat::csgo
{
	// ReSharper disable once CppInconsistentNaming
	class C_BaseEntity;
}

namespace cheat::hooks::c_base_entity
{
	CHEAT_SETUP_HOOK_PROXY(should_interpolate, bool(csgo::C_BaseEntity::*)( ))
	{
		should_interpolate( );

	protected:
		nstd::address get_target_method_impl( ) const override;
		void callback( ) override;
		load_result load_impl( ) noexcept override;
	};
}
