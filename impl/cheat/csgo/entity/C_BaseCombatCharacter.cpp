
#include "../generated/C_BaseCombatCharacter_cpp"

#include "cheat/csgo/entity/C_BaseCombatWeapon.h"

C_BaseCombatWeapon* C_BaseCombatCharacter::GetActiveWeapon( )
{
	return static_cast<C_BaseCombatWeapon*>(m_hActiveWeapon( ).Get( ));
}
