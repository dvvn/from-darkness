#pragma once

#include <fd/valve/client_side/combat_weapon.h>

#if __has_include(<fd/netvars_generated/C_BaseCombatCharacter_h_inc>)
#include <fd/netvars_generated/C_BaseCombatCharacter_h_inc>
#endif

namespace fd::valve::client_side
{
struct combat_character : animating
{
#if __has_include(<fd/netvars_generated/C_BaseCombatCharacter_h>)
#include <fd/netvars_generated/C_BaseCombatCharacter_h>
#endif

    combat_weapon *active_weapon();
};

} // namespace fd::valve::client_side