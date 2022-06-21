export module fd.csgo.interfaces.C_BaseCombatWeapon;
export import fd.csgo.interfaces.C_BaseAnimating;

export namespace fd::csgo
{
    class C_BaseCombatWeapon : public C_BaseAnimating
    {
      public:
#if __has_include("C_BaseCombatWeapon_generated_h")
#include "C_BaseCombatWeapon_generated_h"
#endif
    };
} // namespace fd::csgo
