#pragma once

#include <fd/valve/base_combat_weapon.h>

namespace fd::valve
{
    struct weapon_cs_base : base_combat_weapon
    {
#if __has_include("C_WeaponCSBase_generated_h")
#include "C_WeaponCSBase_generated_h"
#endif
    };
} // namespace fd::valve
