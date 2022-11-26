#pragma once

#include <fd/valve/base_animating.h>

namespace fd::valve
{
    struct base_combat_weapon : base_animating
    {
#if __has_include("C_BaseCombatWeapon_generated_h")
#include "C_BaseCombatWeapon_generated_h"
#endif
    };
} // namespace fd::valve
