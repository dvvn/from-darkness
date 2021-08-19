#pragma once

#include "cheat/core/service.h"
#include "cheat/gui/objects/abstract page.h"
#include "cheat/sdk/entity/C_BaseAnimating.h"

namespace cheat::hooks::c_base_animating
{
	class standard_blending_rules final: public service<standard_blending_rules>,
										 public _Detect_hook_holder_t<decltype(&csgo::C_BaseAnimating::StandardBlendingRules)>,
										 service_skipped_on_gui_test
	{
	public :
		standard_blending_rules( );

	protected:
		bool Do_load( ) override;
		utl::address get_target_method_impl()const override;

		void callback(csgo::CStudioHdr* hdr, csgo::Vector pos[], csgo::QuaternionAligned q[], float current_time, int bone_mask) override;
	};
}
