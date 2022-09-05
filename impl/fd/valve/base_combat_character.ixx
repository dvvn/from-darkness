export module fd.valve.base_combat_character;
export import fd.valve.base_combat_weapon;

using namespace fd::valve;

struct base_combat_character : base_animating
{
#if __has_include("C_BaseCombatCharacter_generated_h")
#include "C_BaseCombatCharacter_generated_h"
#endif
    base_combat_weapon* active_weapon();
};

export namespace fd::valve
{
    using ::base_combat_character;
} // namespace fd::valve
