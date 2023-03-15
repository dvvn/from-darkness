#include <fd/utils/functional.h>
#include <fd/valve/base_entity.h>

#if __has_include(<fd/netvars_generated/C_BaseEntity_cpp_inc>)
#include <fd/netvars_generated/C_BaseEntity_cpp_inc>
#endif

namespace fd::valve
{
#if __has_include(<fd/netvars_generated/C_BaseEntity_cpp>)
#define CLASS_NAME base_entity
// ReSharper disable once CppUnusedIncludeDirective
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