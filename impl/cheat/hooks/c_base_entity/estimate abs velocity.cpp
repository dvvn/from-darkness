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
	this->Wait_for<netvars>( );
}

void estimate_abs_velocity::Load( )
{
#if !defined(CHEAT_GUI_TEST)

	const auto offset = _Find_signature("client.dll", "FF 90 ? ? 00 00 F3 0F 10 4C 24 18").add(2).deref(1).value( ) / 4;
	this->target_func_ = method_info::make_member_virtual(bind_front(_Vtable_pointer<C_BaseEntity>,"client.dll", &csgo_interfaces::local_player), offset);

	this->hook( );
	this->enable( );
#endif
}

void estimate_abs_velocity::Callback(Vector& vel)
{
	const auto ent = this->Target_instance( );
	if (const auto& eflags = reinterpret_cast<bitflag<m_iEFlags_t>&>(ent->m_iEFlags( )); eflags.has(EFL_DIRTY_ABSVELOCITY))
	{
		// ReSharper disable once CppInconsistentNaming
		static auto CalcAbsoluteVelocity_fn = []
		{
			const auto addr = _Find_signature("client.dll", "55 8B EC 83 E4 F8 83 EC 1C 53 56 57 8B F9 F7");
			void (C_BaseEntity::*fn)( );
			reinterpret_cast<void*&>(fn) = addr.raw<void>( );
			return fn;
		}( );

		_Call_function(CalcAbsoluteVelocity_fn, ent);
	}

	vel = ent->m_vecAbsVelocity( );

	this->return_value_.set_original_called(true);
}
