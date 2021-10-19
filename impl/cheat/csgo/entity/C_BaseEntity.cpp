#if __has_include("../generated/C_BaseEntity_cpp")
#include "../generated/C_BaseEntity_cpp"
#else
#include "C_BaseEntity.h"
using namespace cheat::csgo;
#endif

#include "cheat/core/csgo interfaces.h"

#include "cheat/csgo/IClientEntityList.hpp"
#include "cheat/core/csgo modules.h"

#include <dhooks/hook_utils.h>

datamap_t* C_BaseEntity::GetDataDescMap( )
{
	return dhooks::_Call_function(&C_BaseEntity::GetDataDescMap, this, 15);
}

datamap_t* C_BaseEntity::GetPredictionDescMap( )
{
	return dhooks::_Call_function(&C_BaseEntity::GetPredictionDescMap, this, 17);
}
