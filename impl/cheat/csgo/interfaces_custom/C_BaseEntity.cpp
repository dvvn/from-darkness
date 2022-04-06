module cheat.csgo.interfaces.C_BaseEntity;

import cheat.netvars;
import dhooks.wrapper;
import nstd.mem.address;

using namespace cheat::csgo;

#if __has_include("C_BaseEntity_generated_cpp")
#include "C_BaseEntity_generated_cpp"
#endif

datamap_t* C_BaseEntity::GetDataDescMap( )
{
	return dhooks::invoke(&C_BaseEntity::GetDataDescMap, static_cast<size_t>(15), this);
}

datamap_t* C_BaseEntity::GetPredictionDescMap( )
{
	return dhooks::invoke(&C_BaseEntity::GetPredictionDescMap, static_cast<size_t>(17), this);
}
