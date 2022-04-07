module;

#include <functional>

module cheat.csgo.interfaces.C_BaseEntity;

import cheat.netvars;
import nstd.mem.address;

using namespace cheat::csgo;

#if __has_include("C_BaseEntity_generated_cpp")
#include "C_BaseEntity_generated_cpp"
#endif

datamap_t* C_BaseEntity::GetDataDescMap( )
{
	const nstd::mem::basic_address vtable_holder = this;
	const auto fn = vtable_holder.deref<1>( )[15].get<decltype(&C_BaseEntity::GetDataDescMap)>( );
	return std::invoke(fn, this);
}

datamap_t* C_BaseEntity::GetPredictionDescMap( )
{
	const nstd::mem::basic_address vtable_holder = this;
	const auto fn = vtable_holder.deref<1>( )[17].get<decltype(&C_BaseEntity::GetPredictionDescMap)>( );
	return std::invoke(fn, this);
}
