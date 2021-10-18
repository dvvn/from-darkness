
#if __has_include("../generated/C_BaseAnimating_cpp")
#include "../generated/C_BaseAnimating_cpp"
#else
#include "C_BaseAnimating.h"
using namespace cheat::csgo;
#endif
#ifndef CHEAT_GUI_TEST
#include "cheat/core/csgo modules.h"
#include <dhooks/hook_utils.h>
#endif

void C_BaseAnimating::UpdateClientSideAnimation( )
{
#ifndef CHEAT_GUI_TEST
	using namespace nstd::address_pipe;
	//224
	static auto fn = CHEAT_FIND_SIG(client, "55 8B EC 51 56 8B F1 80 BE ? ? ? ? ? 74 36", cast<decltype(&C_BaseAnimating::UpdateClientSideAnimation)>);
	dhooks::_Call_function(fn, this);
#endif
}


