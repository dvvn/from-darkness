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
	struct standard_blending_rules final : hook_instance_shared<standard_blending_rules
															   ,__COUNTER__
															  , void(csgo::C_BaseAnimating::*)(csgo::CStudioHdr*, csgo::Vector*, csgo::QuaternionAligned*, float, int)>
	{
		standard_blending_rules( );

	protected:
		nstd::address get_target_method_impl( ) const override;
		void callback(csgo::CStudioHdr* hdr, csgo::Vector pos[], csgo::QuaternionAligned q[], float current_time, int bone_mask) override;
		load_result load_impl( ) noexcept override;
	};
}
