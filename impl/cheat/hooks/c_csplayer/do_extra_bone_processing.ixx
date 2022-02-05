module;

#include "cheat/hooks/base_includes.h"

export module cheat.hooks.c_csplayer:do_extra_bone_processing;
import cheat.hooks.base;
import cheat.csgo.interfaces;

//namespace cheat::csgo
//{
//	class CIKContext;
//	class CBoneBitList;
//	class matrix3x4a_t;
//	class Quaternion;
//	class Vector;
//	class CStudioHdr;
//	class C_BaseAnimating;
//}

namespace cheat::hooks::c_csplayer
{
	export class do_extra_bone_processing final : public hook_base<
		do_extra_bone_processing,
		void(csgo::C_BaseAnimating::*)(csgo::CStudioHdr*, csgo::Vector*, csgo::Quaternion*, csgo::matrix3x4a_t*
									   , csgo::CBoneBitList&, csgo::CIKContext*)>
	{
		do_extra_bone_processing( );

	protected:
		void construct( ) noexcept override;
		bool load( ) noexcept override;
		void callback(csgo::CStudioHdr* studio_hdr, csgo::Vector pos[], csgo::Quaternion q[], csgo::matrix3x4a_t bone_to_world[], csgo::CBoneBitList& bone_computed,
					  csgo::CIKContext* ik_context) override;
	};

}
