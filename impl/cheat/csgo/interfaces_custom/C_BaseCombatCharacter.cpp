module;

#include "cheat/service/basic_includes.h"

module cheat.csgo.interfaces.C_BaseCombatCharacter;
import cheat.netvars_getter;
import nstd.mem.address;

using namespace cheat::csgo;

#if __has_include("C_BaseCombatCharacter_generated_cpp")
#include "C_BaseCombatCharacter_generated_cpp"
#endif

C_BaseCombatWeapon* C_BaseCombatCharacter::GetActiveWeapon( )
{
#if __has_include("C_BaseCombatWeapon_generated_h")
	return static_cast<C_BaseCombatWeapon*>(m_hActiveWeapon( ).Get( ));
#else
	return nullptr;
#endif
}
