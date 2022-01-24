module;

#include "cheat/service/basic_includes.h"

module cheat.csgo.interfaces:C_BaseCombatCharacter;
import cheat.csgo.interfaces;
import cheat.netvars_getter;
import nstd.mem;

using namespace cheat::csgo;

#if __has_include("C_BaseCombatCharacter_generated_cpp")
#include "C_BaseCombatCharacter_generated_cpp"
#endif

C_BaseCombatWeapon* C_BaseCombatCharacter::GetActiveWeapon( )
{
#if __has_include("C_BaseCombatWeapon_generated_h")
	auto& list = services_loader::get( ).deps( ).get<csgo_interfaces>( ).entity_list;
	auto ptr = list->GetClientEntityFromHandle(m_hActiveWeapon( ));//m_hActiveWeapon( ).Get( )
	return static_cast<C_BaseCombatWeapon*>(ptr);
#else
	return nullptr;
#endif
}
