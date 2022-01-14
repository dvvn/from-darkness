module cheat.csgo.interfaces:C_BaseCombatCharacter;

using namespace cheat::csgo;

#if __has_include("C_BaseCombatCharacter_generated_cpp")
#include "C_BaseCombatCharacter_generated_cpp"
#endif

C_BaseCombatWeapon* C_BaseCombatCharacter::GetActiveWeapon( )
{
#if __has_include("C_BaseCombatCharacter_generated_cpp")
	return static_cast<C_BaseCombatWeapon*>(m_hActiveWeapon( ).Get( ));
#else
	return nullptr;
#endif
}
