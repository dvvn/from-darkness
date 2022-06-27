module;

#include <fd/core/assert.h>

module fd.base_combat_character;
import fd.netvars;

using namespace fd::csgo;

#if __has_include("C_BaseCombatCharacter_generated_cpp")
#include "C_BaseCombatCharacter_generated_cpp"
#endif

fd::base_combat_weapon* base_combat_character::active_weapon()
{
#if __has_include("C_BaseCombatWeapon_generated_h")
    return static_cast<fd::base_combat_weapon*>(m_hActiveWeapon().Get());
#else
    FD_ASSERT_UNREACHABLE("Not implemented");
#endif
}
