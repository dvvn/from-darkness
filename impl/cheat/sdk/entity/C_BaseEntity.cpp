#include "C_BaseEntity.h"

using namespace cheat;
using namespace csgo;

auto C_BaseEntity::GetDataDescMap( ) -> datamap_t*
{
	return utl::hooks::call_virtual_class_method(&C_BaseEntity::GetDataDescMap, this, 15);
}

auto C_BaseEntity::GetPredictionDescMap( ) -> datamap_t*
{
	return utl::hooks::call_virtual_class_method(&C_BaseEntity::GetPredictionDescMap, this, 17);
}
