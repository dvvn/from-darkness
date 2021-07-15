#include "../generated/C_BaseCombatCharacter_cpp"

#include "cheat/core/csgo interfaces.h"

#include "cheat/sdk/IClientEntityList.hpp"

C_BaseCombatWeapon* C_BaseCombatCharacter::GetActiveWeapon( )
{
	const auto wpn_handle = this->m_hActiveWeapon( );
	const auto wpn_ent = csgo_interfaces::get( ).entity_list->GetClientEntityFromHandle(wpn_handle);
	return static_cast<C_BaseCombatWeapon*>(wpn_ent);
}
