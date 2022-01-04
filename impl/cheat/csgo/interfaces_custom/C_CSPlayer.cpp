module cheat.csgo.interfaces:C_CSPlayer;

using namespace cheat::csgo;

C_BaseAnimating* C_CSPlayer::GetRagdoll( )
{
#if __has_include("C_CSPlayer_generated.ixx")
	return static_cast<C_BaseAnimating*>(m_hRagdoll( ).Get( ));
#else
	return nullptr;
#endif
}
