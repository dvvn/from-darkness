#if !CHEAT_NETVARS_UPDATING
#include "../generated/C_BaseEntity_cpp"
#else
#include "C_BaseEntity.h"
using namespace cheat::csgo;
#endif

#include <dhooks/helpers.h>

namespace cheat::csgo
{
	struct datamap_t;
}

datamap_t* C_BaseEntity::GetDataDescMap( )
{
	return dhooks::_Call_function(&C_BaseEntity::GetDataDescMap, this, 15);
}

datamap_t* C_BaseEntity::GetPredictionDescMap( )
{
	return dhooks::_Call_function(&C_BaseEntity::GetPredictionDescMap, this, 17);
}
