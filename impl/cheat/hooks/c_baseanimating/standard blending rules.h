#pragma once

#include "cheat/core/service.h"
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
	class standard_blending_rules final: public base<standard_blending_rules, void(csgo::C_BaseAnimating::*)(csgo::CStudioHdr*, csgo::Vector*, csgo::QuaternionAligned*, float, int)>
									   , service_sometimes_skipped
	{
	public:
		standard_blending_rules( );

	protected:
		nstd::address get_target_method_impl( ) const override;
		void          callback(csgo::CStudioHdr* hdr, csgo::Vector pos[], csgo::QuaternionAligned q[], float current_time, int bone_mask) override;
	};
}
