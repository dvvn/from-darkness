#include <fd/utils/functional.h>
#include <fd/valve/base_entity.h>

#if __has_include("C_BaseEntity_generated_cpp")
#include "C_BaseEntity_generated_cpp"
#endif

namespace fd::valve
{
data_map *base_entity::GetDataDescMap()
{
    return vfunc<data_map *>(this, 15);
}

data_map *base_entity::GetPredictionDescMap()
{
    return vfunc<data_map *>(this, 17);
}
}