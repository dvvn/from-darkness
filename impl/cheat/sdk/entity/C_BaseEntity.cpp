#include "../generated/C_BaseEntity_cpp"

datamap_t* C_BaseEntity::GetDataDescMap( )
{
	return utl::hooks::call_virtual_class_method(&C_BaseEntity::GetDataDescMap, this, 15);
}

datamap_t* C_BaseEntity::GetPredictionDescMap( )
{
	return utl::hooks::call_virtual_class_method(&C_BaseEntity::GetPredictionDescMap, this, 17);
}
