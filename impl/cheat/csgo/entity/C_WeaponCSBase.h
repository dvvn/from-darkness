#pragma once
#include "C_BaseCombatWeapon.h"

namespace cheat::csgo
{
	class C_WeaponCSBase: public C_BaseCombatWeapon
	{
	public:
#if __has_include("../generated/C_WeaponCSBase_h")
#include "../generated/C_WeaponCSBase_h"
#endif
	};
}
