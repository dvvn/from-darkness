#include "standard blending rules.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/netvars/netvars.h"
#include "cheat/players/players list.h"
#include "cheat/sdk/ClientClass.hpp"
#include "cheat/sdk/Studio.hpp"
#include "cheat/utils/signature.h"

using namespace cheat;
using namespace hooks;
using namespace c_base_animating;
using namespace utl;
using namespace csgo;

standard_blending_rules::standard_blending_rules( )
{
}

bool standard_blending_rules::Do_load( )
{
#if defined(CHEAT_GUI_TEST)

	return 0;
#else

	using namespace address_pipe;

	this->target_func_ = method_info::make_member_virtual
			(
			 bind_front(_Vtable_pointer<C_BaseAnimating>, "client.dll"),
			 bind_front(_Find_signature, "client.dll", "8D 94 ? ? ? ? ? 52 56 FF 90 ? ? ? ? 8B 47 FC") | add(11) | deref(1) | divide(4) | value
			);

	this->hook( );
	this->enable( );
	return 1;
#endif
}

void standard_blending_rules::Callback(CStudioHdr* hdr, Vector pos[], QuaternionAligned q[], float current_time, int bone_mask)
{
	const auto pl = this->Target_instance( );
	const auto client_class = pl->GetClientClass( );
	//if (client_class->ClassID != ClassId::CCSPlayer)
	//return;

	auto& flags = reinterpret_cast<m_fEffects_t&>(pl->m_fEffects( ));

	/*if (flags.has(m_fEffects_t::EF_NOINTERP))
		return;*/

	flags.add(m_fEffects_t::EF_NOINTERP);
	this->call_original_ex(hdr, pos, q, current_time, bone_mask|BONE_USED_BY_HITBOX);
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
