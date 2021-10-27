#include "../generated/C_CSPlayer_cpp"

C_BaseAnimating* C_CSPlayer::GetRagdoll( )
{
	return static_cast<C_BaseAnimating*>(m_hRagdoll( ).Get( ));
}
