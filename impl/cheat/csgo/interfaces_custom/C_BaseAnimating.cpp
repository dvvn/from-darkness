module;

#include <nstd/mem/signature_includes.h>
#include <dhooks/helpers.h>

module cheat.csgo.interfaces:C_BaseAnimating;
import cheat.csgo.modules;

using namespace cheat::csgo;

void C_BaseAnimating::UpdateClientSideAnimation( )
{
	//224
	static auto fn = csgo_modules::client->find_signature("55 8B EC 51 56 8B F1 80 BE ? ? ? ? ? 74 36").cast<decltype(&C_BaseAnimating::UpdateClientSideAnimation)>( );
	dhooks::_Call_function(fn, this);
}

void C_BaseAnimating::InvalidateBoneCache( )
{
#if __has_include("C_BaseAnimating_generated.ixx")
	auto& time = m_flLastBoneSetupTime( );
	auto& counter = m_iMostRecentModelBoneCounter( );

	time = -FLT_MAX;
	counter = -1;
#else
	runtime_assert("Not implemented");
#endif
}
