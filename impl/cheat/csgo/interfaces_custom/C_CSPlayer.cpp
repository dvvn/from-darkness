module;

module cheat.csgo.interfaces.C_CSPlayer;
import cheat.netvars_getter;
import nstd.mem.address;

using namespace cheat::csgo;

#if __has_include("C_CSPlayer_generated_cpp")
#include "C_CSPlayer_generated_cpp"
#endif

C_BaseAnimating* C_CSPlayer::GetRagdoll( )
{
#if __has_include("C_BaseAnimating_generated_h")
	return static_cast<C_BaseAnimating*>(m_hRagdoll( ).Get( ));
#else
	return nullptr;
#endif
}
