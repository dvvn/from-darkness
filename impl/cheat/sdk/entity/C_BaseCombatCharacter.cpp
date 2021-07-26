#include "../generated/C_BaseCombatCharacter_cpp"

#include "cheat/core/csgo interfaces.h"

#include "cheat/sdk/IClientEntityList.hpp"

C_BaseCombatWeapon* C_BaseCombatCharacter::GetActiveWeapon( )
{
	const auto &wpn_handle = reinterpret_cast<CBaseHandle&>(this->m_hActiveWeapon( ));
	return static_cast<C_BaseCombatWeapon*>(wpn_handle.Get());
}
