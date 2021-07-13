#include "../generated/C_BaseEntity_cpp"

//#include "C_BaseEntity.h"
//using namespace cheat::csgo;

datamap_t* C_BaseEntity::GetDataDescMap( )
{
	return hooks::call_virtual_class_method(&C_BaseEntity::GetDataDescMap, this, 15);
}

datamap_t* C_BaseEntity::GetPredictionDescMap( )
{
	return hooks::call_virtual_class_method(&C_BaseEntity::GetPredictionDescMap, this, 17);
}
