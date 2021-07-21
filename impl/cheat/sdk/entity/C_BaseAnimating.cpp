#include "../generated/C_BaseAnimating_cpp"

void C_BaseAnimating::UpdateClientSideAnimation( )
{
	hooks::_Call_function(&C_BaseAnimating::UpdateClientSideAnimation, this, 223);
}

//template <typename Ret, typename C, typename ...Args>
//	Ret _Call_function2(Ret (__thiscall C::*fn )(Args...), C* instance, std::type_identity_t<Args>...args)
//	{
//		//using fn_t = Ret(__fastcall*)(void*, Args ...);
//		//return detail::_Call_fn_as<fn_t>(fn, instance, args...);
//	}

// ReSharper disable CppParameterNeverUsed

void C_BaseAnimating::DoExtraBoneProcessing(CStudioHdr* studio_hdr, utl::Vector *pos, Quaternion q[], utl::matrix3x4a_t bone_to_world[], CBoneBitList& bone_computed, CIKContext* ik_context)
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
