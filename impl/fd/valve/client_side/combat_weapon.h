#pragma once

#if __has_include(<fd/netvars_generated/C_BaseCombatWeapon_inc>)
#include <fd/netvars_generated/C_BaseCombatWeapon_inc>
#endif

#include <fd/valve/client_side/animating.h>

namespace fd::valve::client_side
{
struct combat_weapon : animating
{
#if __has_include(<fd/netvars_generated/C_BaseCombatWeapon_h>)
#include <fd/netvars_generated/C_BaseCombatWeapon_h>
#endif
};
} // namespace fd::valve::client_side
