#if __has_include("../generated/C_BaseCombatCharacter_cpp")
#include "../generated/C_BaseCombatCharacter_cpp"
#else
#include "C_BaseCombatCharacter.h"
using namespace cheat::csgo;
#endif

#include "cheat/csgo/entity/C_BaseCombatWeapon.h"

C_BaseCombatWeapon* C_BaseCombatCharacter::GetActiveWeapon( )
{
#if __has_include("../generated/C_BaseCombatCharacter_cpp")
	return static_cast<C_BaseCombatWeapon*>(m_hActiveWeapon( ).Get( ));
#else
	return nullptr;
#endif
}
