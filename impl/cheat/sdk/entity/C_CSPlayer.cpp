#if __has_include("../generated/C_CSPlayer_cpp")
#include "../generated/C_CSPlayer_cpp"
#else
#include "C_CSPlayer.h"
using namespace cheat::csgo;
#endif

C_BaseAnimating* C_CSPlayer::GetRagdoll( )
{
#if __has_include("../generated/C_CSPlayer_cpp")
	return static_cast<C_BaseAnimating*>(m_hRagdoll( ).Get( ));
#else
	return 0;
#endif
}
