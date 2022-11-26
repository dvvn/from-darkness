#pragma once

#include <fd/valve/base_combat_weapon.h>

namespace fd::valve
{
    struct base_combat_character : base_animating
    {
#if __has_include("C_BaseCombatCharacter_generated_h")
#include "C_BaseCombatCharacter_generated_h"
#endif
        base_combat_weapon* active_weapon();
    };

} // namespace fd::valve
