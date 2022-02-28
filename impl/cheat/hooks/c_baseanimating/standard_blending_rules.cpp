module;

#include "cheat/hooks/base_includes.h"
#include "cheat/netvars/storage_includes.h"
#include <nstd/enum_tools.h>

module cheat.hooks.c_base_animating:standard_blending_rules;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;
using namespace hooks::c_base_animating;

standard_blending_rules::standard_blending_rules( ) = default;

void standard_blending_rules::construct( ) noexcept
{
	const nstd::mem::basic_address vtable_holder = csgo_modules::client->find_vtable<C_BaseAnimating>( );
	const auto index = csgo_modules::client->find_signature("8D 94 ? ? ? ? ? 52 56 FF 90 ? ? ? ? 8B 47 FC").plus(11).deref<1>( ).divide(4);
	this->set_target_method(vtable_holder.deref<1>( )[index.value]);
}

void standard_blending_rules::callback(CStudioHdr * hdr, Vector pos[], QuaternionAligned q[], float current_time, int bone_mask)
{
#if 0
	const auto pl = this->get_object_instance( );
	const auto client_class = pl->GetClientClass( );
	//if (client_class->ClassID != ClassId::CCSPlayer)
	//return;

	auto& flags = pl->m_fEffects( );

	/*if (flags.has(m_fEffects_t::EF_NOINTERP))
		return;*/

	using namespace nstd::enum_operators;
	flags |= m_fEffects_t::EF_NOINTERP;
	this->call_original_and_store_result(hdr, pos, q, current_time, bone_mask | BONE_USED_BY_HITBOX);
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

#endif
}