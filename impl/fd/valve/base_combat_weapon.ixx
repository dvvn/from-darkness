export module fd.valve.base_combat_weapon;
export import fd.valve.base_animating;

using namespace fd::valve;

struct base_combat_weapon : base_animating
{
#if __has_include("C_BaseCombatWeapon_generated_h")
#include "C_BaseCombatWeapon_generated_h"
#endif
};

export namespace fd::valve
{
    using ::base_combat_weapon;
}
