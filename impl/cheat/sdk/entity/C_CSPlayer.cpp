#include "../generated/C_CSPlayer_cpp"

#include "cheat/core/csgo interfaces.h"

#include "cheat/sdk/IClientEntityList.hpp"

C_BaseAnimating* C_CSPlayer::GetRagdoll( )
{
	return static_cast<C_BaseAnimating*>(m_hRagdoll( ).Get( ));
}
