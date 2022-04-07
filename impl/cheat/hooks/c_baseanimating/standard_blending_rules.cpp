module;

#include <cheat/hooks/instance.h>
#include <nstd/enum_tools.h>

module cheat.hooks.c_base_animating.standard_blending_rules;
import cheat.csgo.modules;
import cheat.csgo.interfaces.C_BaseAnimating;
import cheat.hooks.base;
import nstd.mem.backup;

using namespace cheat;
using namespace csgo;
using namespace hooks;

CHEAT_HOOK_INSTANCE(c_base_animating, standard_blending_rules);

static void* target( ) noexcept
{
	const nstd::mem::basic_address<void> vtable_holder = csgo_modules::client.find_vtable<C_BaseAnimating>( );
	const auto index = csgo_modules::client.find_signature<"8D 94 ? ? ? ? ? 52 56 FF 90 ? ? ? ? 8B 47 FC">( ).plus(11).deref<1>( ).divide(4);
	return vtable_holder.deref<1>( )[index.value];
}

struct replace
{
	void fn(CStudioHdr* hdr, Vector* pos, QuaternionAligned* q, float current_time, int bone_mask) noexcept
	{
#if __has_include(<cheat/csgo/interfaces_custom/C_BaseAnimating_generated_h>)
		const auto pl = reinterpret_cast<C_BaseAnimating*>(this);
		const auto client_class = pl->GetClientClass( );
		//if (client_class->ClassID != ClassId::CCSPlayer)
		//return;

		auto& flags = pl->m_fEffects( );
		const nstd::mem::backup flags_backup = flags;

		/*if (flags.has(m_fEffects_t::EF_NOINTERP))
			return;*/

		using namespace nstd::enum_operators;
		flags |= m_fEffects_t::EF_NOINTERP;
#endif
		return CHEAT_HOOK_CALL_ORIGINAL_MEMBER(hdr, pos, q, current_time, bone_mask /*| BONE_USED_BY_HITBOX*/);

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
};

CHEAT_HOOK_INIT(c_base_animating, standard_blending_rules);