#pragma once

#include "cheat/hooks/base.h"

namespace cheat::csgo
{
	// ReSharper disable once CppInconsistentNaming
	class C_BaseEntity;
}

namespace cheat::hooks::c_base_entity
{
	class should_interpolate final: public hook_base<should_interpolate, bool(csgo::C_BaseEntity::*)( )>
								  , service_sometimes_skipped
	{
	public:
		should_interpolate( );

	protected:
		nstd::address get_target_method_impl( ) const override;
		void          callback( ) override;
	};
}
