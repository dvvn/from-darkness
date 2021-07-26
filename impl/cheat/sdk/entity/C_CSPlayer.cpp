#include "../generated/C_CSPlayer_cpp"

#include "cheat/core/csgo interfaces.h"

#include "cheat/sdk/IClientEntityList.hpp"

C_BaseAnimating* C_CSPlayer::GetRagdoll( )
{
	const auto &ragdoll_handle = reinterpret_cast<CBaseHandle&>(this->m_hRagdoll( ));
	return static_cast<C_BaseAnimating*>(ragdoll_handle.Get());
}
