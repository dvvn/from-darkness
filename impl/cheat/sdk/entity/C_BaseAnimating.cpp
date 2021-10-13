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
	//224
	static auto fn = CHEAT_FIND_SIG(client, "55 8B EC 51 56 8B F1 80 BE ? ? ? ? ? 74 36").cast<decltype(&C_BaseAnimating::UpdateClientSideAnimation)>( );
	dhooks::_Call_function(fn, this);
}

AnimOverlaysArr_t& C_BaseAnimating::GetAnimOverlays()
{
	using namespace nstd::address_pipe;
	static const auto offset = CHEAT_FIND_SIG(client, "8B 87 ? ? ? ? 83 79 04 00 8B", add(2), deref(1));

	auto& layers = nstd::address(this).add(offset).ref<CUtlVector<CAnimationLayer>>( );
	runtime_assert(layers.size( ) == 13);
	return *reinterpret_cast<AnimOverlaysArr_t*>(layers.data( ));
}
