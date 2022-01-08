module;

#include "cheat/hooks/base_includes.h"

export module cheat.hooks.c_base_animating:standard_blending_rules;
import cheat.hooks.base;
import cheat.csgo.interfaces;

//namespace cheat::csgo
//{
//	class QuaternionAligned;
//	class Vector;
//	class C_BaseAnimating;
//	class CStudioHdr;
//}

namespace cheat::hooks::c_base_animating
{
	struct standard_blending_rules final : hook_base<standard_blending_rules, void(csgo::C_BaseAnimating::*)(csgo::CStudioHdr*, csgo::Vector*, csgo::QuaternionAligned*, float, int)>
	{
		standard_blending_rules( );

	protected:
		void load_async( ) noexcept override;
		void* get_target_method( ) const override;
		void callback(csgo::CStudioHdr* hdr, csgo::Vector pos[], csgo::QuaternionAligned q[], float current_time, int bone_mask) override;
	};
}
