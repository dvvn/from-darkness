export module fds.csgo.interfaces.C_WeaponCSBase;
export import fds.csgo.interfaces.C_BaseCombatWeapon;

namespace fds::csgo
{
    class C_WeaponCSBase : public C_BaseCombatWeapon
    {
      public:
#if __has_include("C_WeaponCSBase_generated_h")
#include "C_WeaponCSBase_generated_h"
#endif
    };
} // namespace fds::csgo
