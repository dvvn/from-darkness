#include "../generated/C_BaseAnimating_cpp"

#include "cheat/core/csgo interfaces.h"

void C_BaseAnimating::UpdateClientSideAnimation( )
{
	hooks::_Call_function(&C_BaseAnimating::UpdateClientSideAnimation, this, 223);
}

void C_BaseAnimating::DoExtraBoneProcessing(CStudioHdr*, Vector*, Quaternion [], matrix3x4a_t [], CBoneBitList&, CIKContext*)
{
	runtime_assert("Dont use. Added only for example");
	(void)this;
}

bool C_BaseAnimating::ShouldSkipAnimationFrame(/*float current_time*/)
{
	runtime_assert("Dont use. Added only for example");
	(void)this;
	return true;
}

void C_BaseAnimating::StandardBlendingRules(CStudioHdr*, Vector [], QuaternionAligned [], float, int)
{
	runtime_assert("Dont use. Added only for example");
	(void)this;
}

CUtlVector<CAnimationLayer>& C_BaseAnimating::GetAnimOverlays( )
{
	static const auto offset = find_signature("client.dll", "8B 87 ? ? ? ? 83 79 04 00 8B").add(2).deref(1);

	(void)this;
	return utl::address(this).add(offset).ref<CUtlVector<CAnimationLayer>>( );
}
