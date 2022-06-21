export module fd.csgo.interfaces.C_WeaponCSBase;
export import fd.csgo.interfaces.C_BaseCombatWeapon;

namespace fd::csgo
{
    class C_WeaponCSBase : public C_BaseCombatWeapon
    {
      public:
#if __has_include("C_WeaponCSBase_generated_h")
#include "C_WeaponCSBase_generated_h"
#endif
    };
} // namespace fd::csgo
