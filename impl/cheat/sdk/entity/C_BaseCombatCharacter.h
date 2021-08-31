#pragma once
#include "C_BaseCombatWeapon.h"
#include "C_BaseAnimating.h"

namespace cheat::csgo
{
	class C_BaseCombatCharacter: public C_BaseAnimating
	{
	public:
#if __has_include("../generated/C_BaseCombatCharacter_h")
#include "../generated/C_BaseCombatCharacter_h"
#endif

		C_BaseCombatWeapon* GetActiveWeapon( );
	};
}
