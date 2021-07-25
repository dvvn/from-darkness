#include "../generated/C_BaseEntity_cpp"

#include "cheat/core/csgo interfaces.h"
#include "cheat/sdk/IClientEntityList.hpp"

//#include "C_BaseEntity.h"
using namespace utl;

datamap_t* C_BaseEntity::GetDataDescMap( )
{
	return hooks::_Call_function(&C_BaseEntity::GetDataDescMap, this, 15);
}

datamap_t* C_BaseEntity::GetPredictionDescMap( )
{
	return hooks::_Call_function(&C_BaseEntity::GetPredictionDescMap, this, 17);
}

void C_BaseEntity::EstimateAbsVelocity([[maybe_unused]] Vector& vel)
{
	BOOST_ASSERT("Dont use. Added only for example");
	(void)this;
}

csgo::CUtlVector<utl::matrix3x4_t>& C_BaseEntity::BonesCache( )
{
	static const auto offset = _Find_signature("client.dll", "8B 55 ? 85 D2 74 23 8B 87 ? ? ? ? 8B 4D ? 3B C8").add(9).deref(1);

	(void)this;
	return utl::address(this).add(offset).remove(8).ref<CUtlVector<matrix3x4_t>>( );
}
