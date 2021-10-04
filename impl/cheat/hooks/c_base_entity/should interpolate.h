#pragma once

#include "cheat/hooks/base.h"

namespace cheat::csgo
{
	// ReSharper disable once CppInconsistentNaming
	class C_BaseEntity;
}

namespace cheat::hooks::c_base_entity
{
	class should_interpolate final: public service_hook_proxy<should_interpolate, bool(csgo::C_BaseEntity::*)( )>
	{
	public:
		should_interpolate( );

	protected:
		nstd::address get_target_method_impl( ) const override;
		void          callback( ) override;
		load_result load_impl() noexcept override;
	};
}
