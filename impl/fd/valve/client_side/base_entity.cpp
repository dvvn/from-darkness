#include <fd/utils/functional.h>
#include <fd/valve/client_side/base_entity.h>

// ReSharper disable CppUnusedIncludeDirective

#if __has_include(<fd/netvars_generated/C_BaseEntity_cpp_inc>)
#define NETVAR_CLASS base_entity
#include <fd/netvars_generated/C_BaseEntity_cpp_inc>
#endif

namespace fd::valve::client_side
{
#if __has_include(<fd/netvars_generated/C_BaseEntity_cpp>)
#include <fd/netvars_generated/C_BaseEntity_cpp>
#endif

data_map *base_entity::GetDataDescMap()
{
    return vfunc<data_map *>(this, 15);
}

data_map *base_entity::GetPredictionDescMap()
{
    return vfunc<data_map *>(this, 17);
}
}