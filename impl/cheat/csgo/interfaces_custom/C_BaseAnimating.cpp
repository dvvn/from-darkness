module;

#include <nstd/mem/signature_includes.h>

module cheat.csgo.interfaces:C_BaseAnimating;
import cheat.csgo.modules;
import cheat.netvars_getter;
import dhooks;
import nstd.mem;

using namespace cheat::csgo;

#if __has_include("C_BaseAnimating_generated_cpp")
#include "C_BaseAnimating_generated_cpp"
#endif

void C_BaseAnimating::UpdateClientSideAnimation( )
{
	//224
	static auto fn = csgo_modules::client->find_signature("55 8B EC 51 56 8B F1 80 BE ? ? ? ? ? 74 36").cast<decltype(&C_BaseAnimating::UpdateClientSideAnimation)>( );
	dhooks::call_function(fn, this);
}

void C_BaseAnimating::InvalidateBoneCache( )
{
#if __has_include("C_BaseAnimating_generated_cpp")
	auto& time = m_flLastBoneSetupTime( );
	auto& counter = m_iMostRecentModelBoneCounter( );

	time = -FLT_MAX;
	counter = -1;
#else
	runtime_assert("Not implemented");
#endif
}
