export module fd.weapon_cs_base;
export import fd.base_combat_weapon;

struct weapon_cs_base : fd::base_combat_weapon
{
#if __has_include("C_WeaponCSBase_generated_h")
#include "C_WeaponCSBase_generated_h"
#endif
};

export namespace fd
{
    using ::weapon_cs_base;
}
