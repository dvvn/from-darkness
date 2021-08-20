#include "../generated/C_BaseEntity_cpp"

#include "cheat/core/csgo interfaces.h"
#include "cheat/sdk/IClientEntityList.hpp"

datamap_t* C_BaseEntity::GetDataDescMap( )
{
	return hooks::_Call_function(&C_BaseEntity::GetDataDescMap, this, 15);
}

datamap_t* C_BaseEntity::GetPredictionDescMap( )
{
	return hooks::_Call_function(&C_BaseEntity::GetPredictionDescMap, this, 17);
}

VarMapping_t* C_BaseEntity::GetInterpVarMap( )
{
	return utl::address(this).add(0x24).ptr<VarMapping_t>( );
}

void C_BaseEntity::EstimateAbsVelocity(Vector&)
{
	CHEAT_UNUSED_ASSERT;
}

bool C_BaseEntity::ShouldInterpolate( )
{
	CHEAT_UNUSED_ASSERT;
	return 1;
}

CUtlVector<matrix3x4_t>& C_BaseEntity::BonesCache( )
{
	static const auto offset = find_signature("client.dll", "8B 55 ? 85 D2 74 23 8B 87 ? ? ? ? 8B 4D ? 3B C8").add(9).deref(1).remove(8);
	(void)this;
	return utl::address(this).add(offset).ref<CUtlVector<matrix3x4_t>>( );
}
