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
	struct do_extra_bone_processing_impl final :
			service<do_extra_bone_processing_impl>
		  , dhooks::_Detect_hook_holder_t<__COUNTER__
										, void(csgo::C_BaseAnimating::*)(csgo::CStudioHdr*, csgo::Vector*, csgo::Quaternion*, csgo::matrix3x4a_t*
																	   , csgo::CBoneBitList&, csgo::CIKContext*)>
	{
		do_extra_bone_processing_impl( );

	protected:
		load_result load_impl( ) noexcept override;
		void* get_target_method( ) const override;
		void callback(csgo::CStudioHdr* studio_hdr, csgo::Vector pos[], csgo::Quaternion q[], csgo::matrix3x4a_t bone_to_world[], csgo::CBoneBitList& bone_computed,
					  csgo::CIKContext* ik_context) override;
	};

	CHEAT_SERVICE_SHARE(do_extra_bone_processing);
}
