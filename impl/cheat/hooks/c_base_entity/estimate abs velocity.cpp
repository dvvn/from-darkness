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

	constexpr auto offset = []
	{
		cheat::detail::csgo_interface_base ifc;
		ifc.from_sig("client.dll", "FF 90 ? ? 00 00 F3 0F 10 4C 24 18", 2, 1);
		return ifc.addr( ).value( ) / 4;
	};

	this->target_func_ = method_info::make_member_virtual(move(get_player), offset( ));

	this->hook( );
	this->enable( );
#endif
}

void estimate_abs_velocity::Callback(utl::Vector& vel)
{
	const auto ent = this->Target_instance( );

	/*const auto client_class = ent->GetClientClass( );
	if (client_class->ClassID != ClassId::CCSPlayer)
		return;*/
	auto& eflags = (reinterpret_cast<bitflag<m_iEFlags_t>&>(ent->m_iEFlags( )));
	if (eflags.has(EFL_DIRTY_ABSANGVELOCITY))
	{
		static auto CalcAbsoluteVelocity_fn = []
		{
			cheat::detail::csgo_interface_base ifc;
			ifc.from_sig("client.dll", "55 8B EC 83 E4 F8 83 EC 1C 53 56 57 8B F9 F7", 0, 0);
			return (ifc.addr( ).value( ));
		}( );

		reinterpret_cast<void(__thiscall *)(C_BaseEntity*)>(CalcAbsoluteVelocity_fn)(ent);

	}

	vel = ent->m_vecAbsVelocity( );

	this->return_value_.set_original_called(true);
}
