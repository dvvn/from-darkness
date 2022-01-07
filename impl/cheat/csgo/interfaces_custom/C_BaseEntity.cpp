module;

#include <functional>

module cheat.csgo.interfaces:C_BaseEntity;
import dhooks;

using namespace cheat::csgo;

datamap_t* C_BaseEntity::GetDataDescMap( )
{
	return dhooks::call_function(&C_BaseEntity::GetDataDescMap, this, 15);
}

datamap_t* C_BaseEntity::GetPredictionDescMap( )
{
	return dhooks::call_function(&C_BaseEntity::GetPredictionDescMap, this, 17);
}
