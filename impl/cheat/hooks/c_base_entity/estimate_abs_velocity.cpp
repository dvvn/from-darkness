#include "estimate_abs_velocity.h"

#include "cheat/core/csgo_modules.h"

#include "cheat/csgo/Vector.hpp"
#include "cheat/csgo/entity/C_BaseEntity.h"

#include <nstd/enum_tools.h>

using namespace cheat;
using namespace csgo;
using namespace hooks::c_base_entity;

void estimate_abs_velocity::callback(Vector& vel)
{
	using namespace nstd::enum_operators;

	const auto ent = this->object_instance;

	if (ent->m_iEFlags( ) & m_iEFlags_t::EFL_DIRTY_ABSVELOCITY)
	{
		static auto calc_absolute_velocity = csgo_modules::client.find_signature<"55 8B EC 83 E4 F8 83 EC 1C 53 56 57 8B F9 F7">( ).cast<void (C_BaseEntity::*)( )>( );
		dhooks::_Call_function(calc_absolute_velocity, ent);
	}

	vel = ent->m_vecAbsVelocity( );

	this->return_value_.set_original_called(true);
}
