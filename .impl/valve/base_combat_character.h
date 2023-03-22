#pragma once

#include <fd/valve/base_combat_weapon.h>

#if __has_include(<fd/netvars_generated/C_BaseCombatCharacter_h_inc>)
#include <fd/netvars_generated/C_BaseCombatCharacter_h_inc>
#endif

namespace fd::valve
{
struct base_combat_character : base_animating
{
#if __has_include(<fd/netvars_generated/C_BaseCombatCharacter_h>)
#include <fd/netvars_generated/C_BaseCombatCharacter_h>
#endif

    base_combat_weapon *active_weapon();
};

} // namespace fd::valve