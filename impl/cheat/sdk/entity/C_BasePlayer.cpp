#include "../generated/C_BasePlayer_cpp"

bool C_BasePlayer::IsAlive( )
{
	return this->m_iHealth( ) > 0;
}
