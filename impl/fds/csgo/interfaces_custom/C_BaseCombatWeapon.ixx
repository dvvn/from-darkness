export module fds.csgo.interfaces.C_BaseCombatWeapon;
export import fds.csgo.interfaces.C_BaseAnimating;

export namespace fds::csgo
{
    class C_BaseCombatWeapon : public C_BaseAnimating
    {
      public:
#if __has_include("C_BaseCombatWeapon_generated_h")
#include "C_BaseCombatWeapon_generated_h"
#endif
    };
} // namespace fds::csgo
