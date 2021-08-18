#include "estimate abs velocity.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/core/helpers.h"
#include "cheat/netvars/netvars.h"
#include "cheat/players/players list.h"
#include "cheat/sdk/ClientClass.hpp"
#include "cheat/sdk/IClientEntityList.hpp"

using namespace cheat;
using namespace hooks;
using namespace c_base_entity;
using namespace utl;
using namespace csgo;

estimate_abs_velocity::estimate_abs_velocity( )
{
}

bool estimate_abs_velocity::Do_load( )
{
	using namespace address_pipe;

	this->target_func_ = method_info::make_member_virtual
			(
			 std::bind_front(_Vtable_pointer<C_BaseEntity>, "client.dll"),
			 std::bind_front(_Find_signature, "client.dll", "FF 90 ? ? 00 00 F3 0F 10 4C 24 18") | add(2) | deref(1) | divide(4) | value
			);

	this->hook( );
	this->enable( );
	return 1;
}

void estimate_abs_velocity::Callback(Vector& vel)
{
	const auto ent = this->Target_instance( );
	if (m_iEFlags_t(ent->m_iEFlags( )).has(m_iEFlags_t::EFL_DIRTY_ABSVELOCITY))
	{
		// ReSharper disable once CppInconsistentNaming
		static auto CalcAbsoluteVelocity_fn = []
		{
			const auto           addr = _Find_signature("client.dll", "55 8B EC 83 E4 F8 83 EC 1C 53 56 57 8B F9 F7");
			void (C_BaseEntity::*fn)( );
			reinterpret_cast<void*&>(fn) = addr.ptr<void>( );
			return fn;
		}( );

		_Call_function(CalcAbsoluteVelocity_fn, ent);
	}

	vel = ent->m_vecAbsVelocity( );

	this->return_value_.set_original_called(true);
}
