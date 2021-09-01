#if __has_include("../generated/C_BaseAnimating_cpp")
#include "../generated/C_BaseAnimating_cpp"
#else
#include "C_BaseAnimating.h"
using namespace cheat::csgo;
#endif

#include "cheat/core/csgo modules.h"

#include <detour hook/hook_utils.h>

void C_BaseAnimating::UpdateClientSideAnimation( )
{
	dhooks::_Call_function(&C_BaseAnimating::UpdateClientSideAnimation, this, 223);
}

CUtlVector<CAnimationLayer>& C_BaseAnimating::GetAnimOverlays( )
{
	static const auto offset = csgo_modules::client.find_signature<"8B 87 ? ? ? ? 83 79 04 00 8B">( ).add(2).deref(1);

	(void)this;
	return nstd::address(this).add(offset).ref<CUtlVector<CAnimationLayer>>( );
}
