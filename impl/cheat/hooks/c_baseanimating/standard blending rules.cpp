#include "standard blending rules.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/core/csgo modules.h"
#include "cheat/core/services loader.h"

#include "cheat/sdk/Studio.hpp"
#include "cheat/sdk/entity/C_BaseAnimating.h"

// ReSharper disable once CppUnusedIncludeDirective
#include "cheat/netvars/config.h"
#include "cheat/netvars/netvars.h"

#include <nstd/enum_tools.h>

using namespace cheat;
using namespace hooks;
using namespace c_base_animating;

using namespace csgo;

standard_blending_rules::standard_blending_rules( )
	: service_maybe_skipped(
#if defined(CHEAT_GUI_TEST) || defined(CHEAT_NETVARS_UPDATING)
								true
#else
								false
#endif
							   )
{
		this->wait_for_service<netvars>( );
}

nstd::address standard_blending_rules::get_target_method_impl( ) const
{
	const auto vtable = csgo_modules::client.find_vtable<C_BaseAnimating>( );
	const auto index  = csgo_modules::client.find_signature<"8D 94 ? ? ? ? ? 52 56 FF 90 ? ? ? ? 8B 47 FC">( ).add(11).deref(1).divide(4).value( );

	return dhooks::_Pointer_to_virtual_class_table(vtable)[index];
}

void standard_blending_rules::callback(CStudioHdr* hdr, Vector pos[], QuaternionAligned q[], float current_time, int bone_mask)
{
#if !__has_include("cheat/sdk/generated/C_BaseEntity_h")
#pragma message(__FUNCTION__ ": skipped")
#else
	const auto pl           = this->object_instance;
	const auto client_class = pl->GetClientClass( );
	//if (client_class->ClassID != ClassId::CCSPlayer)
	//return;

	auto& flags = (pl->m_fEffects( ));

	/*if (flags.has(m_fEffects_t::EF_NOINTERP))
		return;*/

	flags |= nstd::unwrap_enum(m_fEffects_t::EF_NOINTERP);
	this->call_original_ex(hdr, pos, q, current_time, bone_mask | BONE_USED_BY_HITBOX);
	flags &= ~nstd::unwrap_enum(m_fEffects_t::EF_NOINTERP);

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