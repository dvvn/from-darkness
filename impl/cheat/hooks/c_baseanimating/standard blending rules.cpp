#include "standard blending rules.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/netvars/netvars.h"
#include "cheat/players/players list.h"
#include "cheat/sdk/ClientClass.hpp"
#include "cheat/utils/signature.h"

using namespace cheat;
using namespace hooks;
using namespace c_base_animating;
using namespace utl;
using namespace csgo;

standard_blending_rules::standard_blending_rules( )
{
	this->Wait_for<netvars>( );
}

void standard_blending_rules::Load( )
{
#if !defined(CHEAT_GUI_TEST)

	auto get_player = []( )-> C_BaseAnimating*
	{
		if (csgo_interfaces::get( ).local_player != nullptr)
			return csgo_interfaces::get( ).local_player;
		//netvars load it before
		const auto client_dll = all_modules::get( ).find("client.dll");
		const auto& vtables = client_dll->vtables( );
		return vtables.get_cache( ).at("C_BaseAnimating").addr.raw<C_BaseAnimating>( );
	};

	constexpr auto offset = []
	{
		cheat::detail::csgo_interface_base ifc;
		ifc.from_sig("client.dll", "8D 94 ? ? ? ? ? 52 56 FF 90 ? ? ? ? 8B 47 FC", 11, 1);
		return ifc.addr( ).value( ) / 4;
	};

	this->target_func_ = method_info::make_member_virtual(move(get_player), offset( ));

	this->hook( );
	this->enable( );
#endif
}

void standard_blending_rules::Callback(csgo::CStudioHdr* hdr, utl::Vector pos[], csgo::QuaternionAligned q[], float current_time, int bone_mask)
{
	const auto pl = this->Target_instance( );
	const auto client_class = pl->GetClientClass( );
	//if (client_class->ClassID != ClassId::CCSPlayer)
	//return;

	auto& flags = reinterpret_cast<bitflag<m_fEffects_t>&>(pl->m_fEffects( ));

	if (flags.has(EF_NOINTERP))
		return;

	flags.add(m_fEffects_t::EF_NOINTERP);
	this->call_original_ex(hdr, pos, q, current_time, bone_mask);
	flags.remove(m_fEffects_t::EF_NOINTERP);

	/*if (override_return__)
		this->return_value_.store_value(override_return_to__);
	else
	{
		const auto pl = this->Target_instance( );
		const auto client_class = pl->GetClientClass( );
		if (client_class->ClassID != ClassId::CCSPlayer)
			return;

		const auto animate_this_frame = pl->m_bClientSideAnimation( );
		const auto skip_this_frame = animate_this_frame == false;
		this->return_value_.store_value(skip_this_frame);

		(void)client_class;
	}*/
}
