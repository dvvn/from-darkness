#include <fd/valve/base_combat_character.h>

// ReSharper disable CppUnusedIncludeDirective

#if __has_include(<fd/netvars_generated/C_BaseCombatCharacter_cpp_inc>)
#define NETVAR_CLASS base_combat_character
#include <fd/netvars_generated/C_BaseCombatCharacter_cpp_inc>
#endif

namespace fd::valve
{
#if __has_include(<fd/netvars_generated/C_BaseCombatCharacter_cpp>)
#include <fd/netvars_generated/C_BaseCombatCharacter_cpp>
#endif

base_combat_weapon *base_combat_character::active_weapon()
{
    return static_cast<base_combat_weapon *>(m_hActiveWeapon().Get());
}
}