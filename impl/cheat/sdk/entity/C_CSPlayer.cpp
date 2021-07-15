#include "../generated/C_CSPlayer_cpp"

#include "cheat/core/csgo interfaces.h"

#include "cheat/sdk/IClientEntityList.hpp"

C_BaseAnimating* C_CSPlayer::GetRagdoll( )
{
	const auto ragdoll_handle = this->m_hRagdoll( );
	const auto ragdoll_ent = (csgo_interfaces::get( ).entity_list->GetClientEntityFromHandle(ragdoll_handle));
	return static_cast<C_BaseAnimating*>(ragdoll_ent);
}
