#pragma once

#if __has_include(<fd/netvars_generated/C_BaseCombatWeapon_inc>)
#include <fd/netvars_generated/C_BaseCombatWeapon_inc>
#endif

#include <fd/valve/base_animating.h>

namespace fd::valve
{
struct base_combat_weapon : base_animating
{
#if __has_include(<fd/netvars_generated/C_BaseCombatWeapon_h>)
#include <fd/netvars_generated/C_BaseCombatWeapon_h>
#endif
};
} // namespace fd::valve
