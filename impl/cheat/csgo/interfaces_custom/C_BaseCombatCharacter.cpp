module cheat.csgo.interfaces:C_BaseCombatCharacter;

using namespace cheat::csgo;

C_BaseCombatWeapon* C_BaseCombatCharacter::GetActiveWeapon( )
{
#if __has_include("C_BaseCombatCharacter_generated.ixx")
	return static_cast<C_BaseCombatWeapon*>(m_hActiveWeapon( ).Get( ));
#else
	return nullptr;
#endif
}
