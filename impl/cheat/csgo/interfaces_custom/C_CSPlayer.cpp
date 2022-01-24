module;

#include "cheat/service/basic_includes.h"

module cheat.csgo.interfaces:C_CSPlayer;

import cheat.csgo.interfaces;
import cheat.netvars_getter;
import nstd.mem;

using namespace cheat::csgo;

#if __has_include("C_CSPlayer_generated_cpp")
#include "C_CSPlayer_generated_cpp"
#endif

C_BaseAnimating* C_CSPlayer::GetRagdoll( )
{
#if __has_include("C_BaseAnimating_generated_h")
	auto& list = services_loader::get( ).deps( ).get<csgo_interfaces>( ).entity_list;
	auto ptr = list->GetClientEntityFromHandle(m_hRagdoll( ));//m_hRagdoll( )( ).Get( )
	return static_cast<C_BaseAnimating*>(ptr);
#else
	return nullptr;
#endif
}
