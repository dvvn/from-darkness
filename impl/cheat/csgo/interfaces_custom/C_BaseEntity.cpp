module cheat.csgo.interfaces:C_BaseEntity;

import cheat.netvars_getter;
import dhooks;
import nstd.mem.address;

using namespace cheat::csgo;

#if __has_include("C_BaseEntity_generated_cpp")
#include "C_BaseEntity_generated_cpp"
#endif

datamap_t* C_BaseEntity::GetDataDescMap( )
{
	return dhooks::call_function(&C_BaseEntity::GetDataDescMap, this, 15);
}

datamap_t* C_BaseEntity::GetPredictionDescMap( )
{
	return dhooks::call_function(&C_BaseEntity::GetPredictionDescMap, this, 17);
}
