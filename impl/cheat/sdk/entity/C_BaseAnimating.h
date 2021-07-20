#pragma once
#include "C_BaseEntity.h"

namespace cheat::csgo
{
	class CStudioHdr;
	class Quaternion;
	class CBoneBitList;
	class CIKContext;

	class C_BaseAnimating: public C_BaseEntity
	{
	public:
#include "../generated/C_BaseAnimating_h"

		void UpdateClientSideAnimation( );

		void DoExtraBoneProcessing(CStudioHdr* studio_hdr, utl::Vector pos[], Quaternion q[], utl::matrix3x4a_t bone_to_world[], CBoneBitList& bone_computed, CIKContext* ik_context);
		bool ShouldSkipAnimationFrame(/*float current_time*/);
	};
}
