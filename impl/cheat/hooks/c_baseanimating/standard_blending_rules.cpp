#include "standard_blending_rules.h"

#include "cheat/csgo/Studio.hpp"
#include "cheat/csgo/entity/C_BaseAnimating.h"

#include <nstd/enum_tools.h>

using namespace cheat;
using namespace csgo;
using namespace hooks::c_base_animating;

void standard_blending_rules::callback(CStudioHdr* hdr, Vector pos[], QuaternionAligned q[], float current_time, int bone_mask)
{
	const auto pl           = this->object_instance;
	const auto client_class = pl->GetClientClass( );
	//if (client_class->ClassID != ClassId::CCSPlayer)
	//return;

	auto& flags = pl->m_fEffects( );

	/*if (flags.has(m_fEffects_t::EF_NOINTERP))
		return;*/

	using namespace nstd::enum_operators;
	flags |= m_fEffects_t::EF_NOINTERP;
	this->call_original_ex(hdr, pos, q, current_time, bone_mask | BONE_USED_BY_HITBOX);
	flags &= ~m_fEffects_t::EF_NOINTERP;

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
