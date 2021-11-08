#include "../generated/C_BaseAnimating_cpp"

#include "cheat/core/csgo_modules.h"

#include <dhooks/helpers.h>

void C_BaseAnimating::UpdateClientSideAnimation( )
{
	//224
	static auto fn = csgo_modules::client->find_signature("55 8B EC 51 56 8B F1 80 BE ? ? ? ? ? 74 36").cast<decltype(&C_BaseAnimating::UpdateClientSideAnimation)>( );
	dhooks::_Call_function(fn, this);
}

void C_BaseAnimating::InvalidateBoneCache( )
{
	auto& time    = m_flLastBoneSetupTime( );
	auto& counter = m_iMostRecentModelBoneCounter( );

	time    = -FLT_MAX;
	counter = -1;
}
