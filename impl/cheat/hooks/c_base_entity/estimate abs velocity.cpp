#include "estimate abs velocity.h"

#include "cheat/core/console.h"
#include "cheat/core/services loader.h"
#include "cheat/core/csgo modules.h"

#include "cheat/csgo/Vector.hpp"
#include "cheat/csgo/entity/C_BaseEntity.h"

#include "cheat/netvars/config.h"
#include "cheat/netvars/netvars.h"

#include "nstd/enum_tools.h"

using namespace cheat;
using namespace csgo;
using namespace hooks::c_base_entity;

#ifndef CHEAT_GUI_TEST
using namespace nstd::address_pipe;
#endif

estimate_abs_velocity::estimate_abs_velocity( )
{
	this->wait_for_service<netvars>( );
}

CHEAT_HOOK_PROXY_INIT_FN(estimate_abs_velocity, CHEAT_MODE_INGAME)
CHEAT_HOOK_PROXY_TARGET_FN(estimate_abs_velocity, CHEAT_MODE_INGAME,
						   CHEAT_FIND_VTABLE(client, C_BaseEntity),
						   CHEAT_FIND_SIG(client, "FF 90 ? ? 00 00 F3 0F 10 4C 24 18", add(2), deref(1), divide(4)));

void estimate_abs_velocity::callback(Vector& vel)
{
#if !CHEAT_MODE_INGAME || !__has_include("cheat/csgo/generated/C_BaseEntity_h")
	CHEAT_CALL_BLOCKER
#else
	using namespace nstd::enum_operators;

	const auto ent = this->object_instance;

	if (ent->m_iEFlags() & m_iEFlags_t::EFL_DIRTY_ABSVELOCITY)
	{
		static auto calc_absolute_velocity = CHEAT_FIND_SIG(client, "55 8B EC 83 E4 F8 83 EC 1C 53 56 57 8B F9 F7", cast<void (C_BaseEntity::*)()>);
		dhooks::_Call_function(calc_absolute_velocity, ent);
	}

	vel = ent->m_vecAbsVelocity();

	this->return_value_.set_original_called(true);
#endif
}

CHEAT_REGISTER_SERVICE(estimate_abs_velocity);
