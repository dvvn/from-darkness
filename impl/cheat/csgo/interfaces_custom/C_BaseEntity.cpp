module;

#include <dhooks/helpers.h>

module cheat.csgo.interfaces:C_BaseEntity;

using namespace cheat::csgo;

datamap_t* C_BaseEntity::GetDataDescMap( )
{
	return dhooks::_Call_function(&C_BaseEntity::GetDataDescMap, this, 15);
}

datamap_t* C_BaseEntity::GetPredictionDescMap( )
{
	return dhooks::_Call_function(&C_BaseEntity::GetPredictionDescMap, this, 17);
}
