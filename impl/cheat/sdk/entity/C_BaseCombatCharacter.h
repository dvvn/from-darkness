#pragma once
#include "C_BaseCombatWeapon.h"
#include "C_BaseFlex.h"

namespace cheat::csgo
{
	class C_BaseCombatCharacter: public C_BaseFlex
	{
	public:
#include "../generated/C_BaseCombatCharacter_h"

		C_BaseCombatWeapon* GetActiveWeapon( );
	};
}
