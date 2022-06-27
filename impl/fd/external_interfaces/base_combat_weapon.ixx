export module fd.base_combat_weapon;
export import fd.base_animating;

struct base_combat_weapon : fd::base_animating
{
#if __has_include("C_BaseCombatWeapon_generated_h")
#include "C_BaseCombatWeapon_generated_h"
#endif
};

export namespace fd
{
    using ::base_combat_weapon;
}
