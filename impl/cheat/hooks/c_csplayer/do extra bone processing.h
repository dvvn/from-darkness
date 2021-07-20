#pragma once

#include "cheat/core/service.h"
#include "cheat/sdk/entity/C_BaseAnimating.h"

namespace cheat::hooks::c_csplayer
{
	class do_extra_bone_processing final: public service_shared<do_extra_bone_processing, service_mode::async>,
										  public decltype(detect_hook_holder(&csgo::C_BaseAnimating::DoExtraBoneProcessing))
	{
	public :
		do_extra_bone_processing( );

	protected:
		void Load( ) override;
		void Callback(csgo::CStudioHdr* studio_hdr,
					  utl::Vector pos[], csgo::Quaternion q[], utl::matrix3x4a_t bone_to_world[],
					  csgo::CBoneBitList& bone_computed, csgo::CIKContext* ik_context) override;
	};
}
