#if __has_include("../generated/C_BasePlayer_cpp")
#include "../generated/C_BasePlayer_cpp"
#else
#include "C_BasePlayer.h"
using namespace cheat::csgo;
#endif

bool C_BasePlayer::IsAlive( )
{
#if __has_include("../generated/C_BasePlayer_cpp")
	return this->m_iHealth( ) > 0;
#else
	return false;
#endif
}
