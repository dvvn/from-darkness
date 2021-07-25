#include "../generated/C_BaseAnimating_cpp"

#include "cheat/core/csgo interfaces.h"

void C_BaseAnimating::UpdateClientSideAnimation( )
{
	hooks::_Call_function(&C_BaseAnimating::UpdateClientSideAnimation, this, 223);
}

// ReSharper disable CppParameterNeverUsed

void C_BaseAnimating::DoExtraBoneProcessing(CStudioHdr* studio_hdr, utl::Vector* pos, Quaternion q[], utl::matrix3x4a_t bone_to_world[], CBoneBitList& bone_computed, CIKContext* ik_context)
{
	BOOST_ASSERT("Dont use. Added only for example");
	(void)this;
}

bool C_BaseAnimating::ShouldSkipAnimationFrame(/*float current_time*/)
{
	BOOST_ASSERT("Dont use. Added only for example");
	(void)this;
	return true;
}

void C_BaseAnimating::StandardBlendingRules(CStudioHdr* hdr, utl::Vector pos[], QuaternionAligned q[], float current_time, int bone_mask)
{
	BOOST_ASSERT("Dont use. Added only for example");
	(void)this;
}

// ReSharper restore CppParameterNeverUsed

CUtlVector<CAnimationLayer>& C_BaseAnimating::GetAnimOverlays( )
{
	static const auto offset = _Find_signature("client.dll", "8B 87 ? ? ? ? 83 79 04 00 8B").add(2).deref(1);

	(void)this;
	return utl::address(this).add(offset).ref<CUtlVector<CAnimationLayer>>( );
}
