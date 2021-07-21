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
