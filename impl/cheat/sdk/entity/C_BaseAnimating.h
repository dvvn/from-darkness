#pragma once
#include "C_BaseEntity.h"

namespace cheat::csgo
{
	class C_BaseAnimating: public C_BaseEntity
	{
	public:
#include "../generated/C_BaseAnimating_h"

		void UpdateClientSideAnimation( );
	};
}
