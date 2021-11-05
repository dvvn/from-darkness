#pragma once

#include "cheat/hooks/base.h"

namespace cheat::csgo
{
	class QuaternionAligned;
	class Vector;
	class C_BaseAnimating;
	class CStudioHdr;
}

namespace cheat::hooks::c_base_animating
{
	struct standard_blending_rules_impl final : service<standard_blending_rules_impl>
											  , dhooks::_Detect_hook_holder_t
												<__COUNTER__, void(csgo::C_BaseAnimating::*)(csgo::CStudioHdr*, csgo::Vector*, csgo::QuaternionAligned*, float, int)>
	{
		standard_blending_rules_impl( );

	protected:
		void* get_target_method( ) const override;
		void callback(csgo::CStudioHdr* hdr, csgo::Vector pos[], csgo::QuaternionAligned q[], float current_time, int bone_mask) override;
		load_result load_impl( ) noexcept override;
	};

	CHEAT_SERVICE_SHARE(standard_blending_rules);
}
