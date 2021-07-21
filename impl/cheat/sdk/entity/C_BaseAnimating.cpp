#include "../generated/C_BaseAnimating_cpp"

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

// ReSharper restore CppParameterNeverUsed
