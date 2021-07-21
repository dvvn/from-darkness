#include "estimate abs velocity.h"

#include "cheat/core/csgo interfaces.h"
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

	auto get_player = []( )-> C_BaseEntity*
	{
		if (csgo_interfaces::get( ).local_player != nullptr)
			return csgo_interfaces::get( ).local_player;
		//netvars load it before
		const auto client_dll = all_modules::get( ).find("client.dll");
		const auto& vtables = client_dll->vtables( );
		return vtables.get_cache( ).at("C_BaseEntity").addr.raw<C_BaseEntity>( );
	};
	auto get_player2 = []( )-> C_BasePlayer*
	{
		if (csgo_interfaces::get( ).local_player != nullptr)
			return csgo_interfaces::get( ).local_player;
		//netvars load it before
		const auto client_dll = all_modules::get( ).find("client.dll");
		const auto& vtables = client_dll->vtables( );
		return vtables.get_cache( ).at("C_BasePlayer").addr.raw<C_BasePlayer>( );
	};

	constexpr auto offset = []
	{
		cheat::detail::csgo_interface_base ifc;
		ifc.from_sig("client.dll", "FF 90 ? ? 00 00 F3 0F 10 4C 24 18", 2, 1);
		return ifc.addr( ).value( ) / 4;
	};

	auto vtable1 = get_player( );
	auto vtable2 = get_player( );

	this->target_func_ = method_info::make_member_virtual(vtable1, offset( ));

	this->hook( );
	this->enable( );
#endif
}

void estimate_abs_velocity::Callback(Vector& vel)
{
	const auto ent = this->Target_instance( );
	if (const auto& eflags = reinterpret_cast<bitflag<m_iEFlags_t>&>(ent->m_iEFlags( )); eflags.has(EFL_DIRTY_ABSVELOCITY))
	{
		static auto CalcAbsoluteVelocity_fn = []
		{
			cheat::detail::csgo_interface_base ifc;
			ifc.from_sig("client.dll", "55 8B EC 83 E4 F8 83 EC 1C 53 56 57 8B F9 F7", 0, 0);
			void (C_BaseEntity::*fn)( );
			reinterpret_cast<void*&>(fn) = ifc.addr( ).raw<void>( );
			return fn;
		}( );

		_Call_function(CalcAbsoluteVelocity_fn, ent);
	}

	vel = ent->m_vecAbsVelocity( );

	this->return_value_.set_original_called(true);
}
