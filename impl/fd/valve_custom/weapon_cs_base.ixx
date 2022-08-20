export module fd.valve.weapon_cs_base;
export import fd.valve.base_combat_weapon;

using namespace fd::valve;

struct weapon_cs_base : base_combat_weapon
{
#if __has_include("C_WeaponCSBase_generated_h")
#include "C_WeaponCSBase_generated_h"
#endif
};

export namespace fd::valve
{
    using ::weapon_cs_base;
}
