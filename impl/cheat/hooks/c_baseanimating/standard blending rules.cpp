#include "standard blending rules.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo interfaces.h"
#include "cheat/core/csgo modules.h"
#include "cheat/core/services loader.h"

#include "cheat/netvars/config.h"
#include "cheat/netvars/netvars.h"

#include "cheat/sdk/Studio.hpp"
#include "cheat/sdk/entity/C_BaseAnimating.h"

#include <nstd/enum_tools.h>

using namespace cheat;
using namespace csgo;
using namespace hooks::c_base_animating;

#ifndef CHEAT_GUI_TEST
using namespace nstd::address_pipe;
#endif

standard_blending_rules::standard_blending_rules()
{
	this->wait_for_service<netvars>( );
}

CHEAT_HOOK_PROXY_INIT_FN(standard_blending_rules, CHEAT_MODE_INGAME)
CHEAT_HOOK_PROXY_TARGET_FN(standard_blending_rules, CHEAT_MODE_INGAME,
						   CHEAT_FIND_VTABLE(client, C_BaseAnimating),
						   CHEAT_FIND_SIG(client, "8D 94 ? ? ? ? ? 52 56 FF 90 ? ? ? ? 8B 47 FC", add(11), deref(1), divide(4), value));

void standard_blending_rules::callback(CStudioHdr* hdr, Vector pos[], QuaternionAligned q[], float current_time, int bone_mask)
{
#if !CHEAT_MODE_INGAME || !__has_include("cheat/sdk/generated/C_BaseEntity_h")
	CHEAT_CALL_BLOCKER
#else
	const auto pl           = this->object_instance;
	const auto client_class = pl->GetClientClass( );
	//if (client_class->ClassID != ClassId::CCSPlayer)
	//return;

	auto& flags = (pl->m_fEffects( ));

	/*if (flags.has(m_fEffects_t::EF_NOINTERP))
		return;*/

	using namespace nstd::enum_operators;
	flags |= (m_fEffects_t::EF_NOINTERP);
	this->call_original_ex(hdr, pos, q, current_time, bone_mask | BONE_USED_BY_HITBOX);
	flags &= ~(m_fEffects_t::EF_NOINTERP);

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
#endif
}

CHEAT_REGISTER_SERVICE(standard_blending_rules);
