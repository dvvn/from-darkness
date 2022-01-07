module;

#include "cheat/hooks/base_includes.h"
#include "cheat/netvars/includes.h"
#include "cheat/csgo/modules_includes.h"

#include <nstd/enum_tools.h>

module cheat.hooks.c_base_entity:estimate_abs_velocity;
import cheat.netvars;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;
using namespace hooks::c_base_entity;

estimate_abs_velocity::estimate_abs_velocity( )
{
	this->add_dependency(netvars::get( ));
}

void* estimate_abs_velocity::get_target_method( ) const
{
	const csgo_interface vtable = csgo_modules::client->find_vtable<C_BaseEntity>( );
	const auto index = csgo_modules::client->find_signature("FF 90 ? ? 00 00 F3 0F 10 4C 24 18").add(2).deref(1).divide(4)._Unwrap<uintptr_t>( );
	return vtable.vfunc(index).ptr( );
}

void estimate_abs_velocity::callback(Vector& vel)
{
#if 0
	using namespace nstd::enum_operators;

	const auto ent = this->get_object_instance( );

	if (ent->m_iEFlags( ) & m_iEFlags_t::EFL_DIRTY_ABSVELOCITY)
	{
		static auto calc_absolute_velocity = csgo_modules::client->find_signature("55 8B EC 83 E4 F8 83 EC 1C 53 56 57 8B F9 F7").cast<void (C_BaseEntity::*)()>( );
		dhooks::call_function(calc_absolute_velocity, ent);
	}

	vel = ent->m_vecAbsVelocity( );

	this->store_return_value(true);
#endif
}

CHEAT_SERVICE_REGISTER_GAME(estimate_abs_velocity);
