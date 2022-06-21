export module fd.csgo.interfaces.C_BaseCombatCharacter;
export import fd.csgo.interfaces.C_BaseAnimating;

export namespace fd::csgo
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
} // namespace fd::csgo
