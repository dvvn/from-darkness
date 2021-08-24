#include "standard blending rules.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/players/players list.h"
#include "cheat/sdk/ClientClass.hpp"
#include "cheat/sdk/Studio.hpp"

using namespace cheat;
using namespace hooks;
using namespace c_base_animating;

using namespace csgo;

nstd::address standard_blending_rules::get_target_method_impl( ) const
{
	const auto vtable = vtable_pointer<C_BaseAnimating>("client.dll");
	const auto index  = find_signature("client.dll", "8D 94 ? ? ? ? ? 52 56 FF 90 ? ? ? ? 8B 47 FC").add(11).deref(1).divide(4).value( );

	return dhooks::_Pointer_to_virtual_class_table(vtable)[index];
}

void standard_blending_rules::callback(CStudioHdr* hdr, Vector pos[], QuaternionAligned q[], float current_time, int bone_mask)
{
	const auto pl           = this->object_instance;
	const auto client_class = pl->GetClientClass( );
	//if (client_class->ClassID != ClassId::CCSPlayer)
	//return;

	auto& flags = reinterpret_cast<m_fEffects_t&>(pl->m_fEffects( ));

	/*if (flags.has(m_fEffects_t::EF_NOINTERP))
		return;*/

	flags.add(m_fEffects_t::EF_NOINTERP);
	this->call_original_ex(hdr, pos, q, current_time, bone_mask | BONE_USED_BY_HITBOX);
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
