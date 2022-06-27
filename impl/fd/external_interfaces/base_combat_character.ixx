export module fd.base_combat_character;
export import fd.base_combat_weapon;

struct base_combat_character : fd::base_animating
{
#if __has_include("C_BaseCombatCharacter_generated_h")
#include "C_BaseCombatCharacter_generated_h"
#endif
    fd::base_combat_weapon* active_weapon();
};

export namespace fd
{
    using ::base_combat_character;
} // namespace fd
