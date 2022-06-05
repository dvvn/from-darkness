module;

module fds.csgo.interfaces.C_BaseCombatCharacter;
import fds.netvars;
import nstd.mem.address;

using namespace fds::csgo;

#if __has_include("C_BaseCombatCharacter_generated_cpp")
#include "C_BaseCombatCharacter_generated_cpp"
#endif

C_BaseCombatWeapon* C_BaseCombatCharacter::GetActiveWeapon()
{
#if __has_include("C_BaseCombatWeapon_generated_h")
    return static_cast<C_BaseCombatWeapon*>(m_hActiveWeapon().Get());
#else
    return nullptr;
#endif
}
