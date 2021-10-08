#if __has_include("../generated/C_BaseEntity_cpp")
#include "../generated/C_BaseEntity_cpp"
#else
#include "C_BaseEntity.h"
using namespace cheat::csgo;
#endif

#include "cheat/core/csgo interfaces.h"
#include "cheat/sdk/IClientEntityList.hpp"
#include "cheat/core/csgo modules.h"

#include <dhooks/hook_utils.h>

datamap_t* C_BaseEntity::GetDataDescMap()
{
	return dhooks::_Call_function(&C_BaseEntity::GetDataDescMap, this, 15);
}

datamap_t* C_BaseEntity::GetPredictionDescMap()
{
	return dhooks::_Call_function(&C_BaseEntity::GetPredictionDescMap, this, 17);
}

VarMapping_t* C_BaseEntity::GetInterpVarMap()
{
	return nstd::address(this).add(0x24).ptr<VarMapping_t>( );
}

CUtlVector<matrix3x4_t>& C_BaseEntity::BonesCache()
{
	using namespace nstd::address_pipe;
	static const auto offset = CHEAT_FIND_SIG(client, "8B 55 ? 85 D2 74 23 8B 87 ? ? ? ? 8B 4D ? 3B C8", add(9), deref(1), remove(8));
	return nstd::address(this).add(offset).ref<CUtlVector<matrix3x4_t>>( );
}
