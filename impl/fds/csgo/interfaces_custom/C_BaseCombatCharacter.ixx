export module fds.csgo.interfaces.C_BaseCombatCharacter;
export import fds.csgo.interfaces.C_BaseAnimating;

export namespace fds::csgo
{
    class C_BaseCombatWeapon;

    class C_BaseCombatCharacter : public C_BaseAnimating
    {
      public:
#if __has_include("C_BaseCombatCharacter_generated_h")
#include "C_BaseCombatCharacter_generated_h"
#endif

        C_BaseCombatWeapon* GetActiveWeapon();
    };
} // namespace fds::csgo
