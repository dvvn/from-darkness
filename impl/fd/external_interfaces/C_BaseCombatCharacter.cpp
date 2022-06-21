module;

#include <fd/core/assert.h>

module fd.csgo.interfaces.C_BaseCombatCharacter;
import fd.netvars;

using namespace fd::csgo;

#if __has_include("C_BaseCombatCharacter_generated_cpp")
#include "C_BaseCombatCharacter_generated_cpp"
#endif

C_BaseCombatWeapon* C_BaseCombatCharacter::GetActiveWeapon()
{
#if __has_include("C_BaseCombatWeapon_generated_h")
    return static_cast<C_BaseCombatWeapon*>(m_hActiveWeapon().Get());
#else
    FD_ASSERT_UNREACHABLE("Not implemented");
#endif
}
