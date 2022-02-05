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
	export class standard_blending_rules final :public hook_base<standard_blending_rules, void(csgo::C_BaseAnimating::*)(csgo::CStudioHdr*, csgo::Vector*, csgo::QuaternionAligned*, float, int)>
	{
	public:
		standard_blending_rules( );

	protected:
		void construct( ) noexcept override;
		bool load( ) noexcept override;
		void callback(csgo::CStudioHdr* hdr, csgo::Vector pos[], csgo::QuaternionAligned q[], float current_time, int bone_mask) override;
	};
}
