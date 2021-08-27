#pragma once

#include "cheat/core/service.h"
#include "cheat/sdk/entity/C_BaseAnimating.h"

namespace cheat::hooks::c_csplayer
{
	class do_extra_bone_processing final: public service<do_extra_bone_processing>
										, public dhooks::_Detect_hook_holder_t<decltype(&csgo::C_BaseAnimating::DoExtraBoneProcessing)>
										, service_hook_helper
#if defined(CHEAT_GUI_TEST) || defined(CHEAT_NETVARS_UPDATING)
										, service_always_skipped
#endif
	{
	public:
		do_extra_bone_processing( );

	protected:
		nstd::address get_target_method_impl( ) const override;
		void          callback(csgo::CStudioHdr* studio_hdr, csgo::Vector pos[], csgo::Quaternion q[], csgo::matrix3x4a_t bone_to_world[], csgo::CBoneBitList& bone_computed,
							   csgo::CIKContext* ik_context) override;
	};
}
