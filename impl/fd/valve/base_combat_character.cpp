module;

#include <fd/assert.h>

module fd.valve.base_combat_character;

#if __has_include("C_BaseCombatCharacter_generated_cpp")
#include "C_BaseCombatCharacter_generated_cpp"
#endif

using namespace fd::valve;

base_combat_weapon* base_combat_character::active_weapon()
{
#if __has_include("C_BaseCombatWeapon_generated_h")
    return static_cast<base_combat_weapon*>(m_hActiveWeapon().Get());
#else
    FD_ASSERT_UNREACHABLE("Not implemented");
#endif
}
