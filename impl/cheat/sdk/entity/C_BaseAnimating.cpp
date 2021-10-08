#if __has_include("../generated/C_BaseAnimating_cpp")
#include "../generated/C_BaseAnimating_cpp"
#else
#include "C_BaseAnimating.h"
using namespace cheat::csgo;
#endif

#include "cheat/core/csgo modules.h"

#include <dhooks/hook_utils.h>

void C_BaseAnimating::UpdateClientSideAnimation()
{
	static auto func = []
	{
		//224
		const auto addr = csgo_modules::client.find_signature<"55 8B EC 51 56 8B F1 80 BE ? ? ? ? ? 74 36">( );
		decltype(&C_BaseAnimating::UpdateClientSideAnimation) fn;
		reinterpret_cast<void*&>(fn) = addr.ptr<void>( );
		return fn;
	}( );

	dhooks::_Call_function(func, this);
}

CUtlVector<CAnimationLayer>& C_BaseAnimating::GetAnimOverlays()
{
	static const auto offset = csgo_modules::client.find_signature<"8B 87 ? ? ? ? 83 79 04 00 8B">( ).add(2).deref(1);

	(void)this;
	return nstd::address(this).add(offset).ref<CUtlVector<CAnimationLayer>>( );
}
