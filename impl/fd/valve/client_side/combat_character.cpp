#include <fd/valve/client_side/combat_character.h>

// ReSharper disable CppUnusedIncludeDirective

#if __has_include(<fd/netvars_generated/C_BaseCombatCharacter_cpp_inc>)
#define NETVAR_CLASS combat_character
#include <fd/netvars_generated/C_BaseCombatCharacter_cpp_inc>
#endif

namespace fd::valve::client_side
{
#if __has_include(<fd/netvars_generated/C_BaseCombatCharacter_cpp>)
#include <fd/netvars_generated/C_BaseCombatCharacter_cpp>
extern entity_list *ents_list;
#endif

combat_weapon *combat_character::active_weapon()
{
#ifdef NETVAR_CLASS
    return static_cast<combat_weapon *>(m_hActiveWeapon().Get());
#else
    std::unreachable();
#endif
}
}