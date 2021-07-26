#include "../generated/C_BaseCombatCharacter_cpp"

#include "cheat/core/csgo interfaces.h"

#include "cheat/sdk/IClientEntityList.hpp"

C_BaseCombatWeapon* C_BaseCombatCharacter::GetActiveWeapon( )
{
	return static_cast<C_BaseCombatWeapon*>(m_hActiveWeapon( ).Get( ));
}
