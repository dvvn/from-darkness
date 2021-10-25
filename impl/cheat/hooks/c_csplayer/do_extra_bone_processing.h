#pragma once

#include "cheat/hooks/base.h"

namespace cheat::csgo
{
	class CIKContext;
	class CBoneBitList;
	class matrix3x4a_t;
	class Quaternion;
	class Vector;
	class CStudioHdr;
	class C_BaseAnimating;
}

namespace cheat::hooks::c_csplayer
{
	struct do_extra_bone_processing final :
			hook_instance_shared<do_extra_bone_processing
							   , __COUNTER__
							   , void(csgo::C_BaseAnimating::*)(csgo::CStudioHdr*, csgo::Vector*, csgo::Quaternion*, csgo::matrix3x4a_t*
															  , csgo::CBoneBitList&, csgo::CIKContext*)>
	{
		do_extra_bone_processing( );

	protected:
		load_result load_impl( ) noexcept override;
		nstd::address get_target_method_impl( ) const override;
		void callback(csgo::CStudioHdr* studio_hdr, csgo::Vector pos[], csgo::Quaternion q[], csgo::matrix3x4a_t bone_to_world[], csgo::CBoneBitList& bone_computed,
					  csgo::CIKContext* ik_context) override;
	};
}
