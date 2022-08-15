module;

module fd.valve.base_entity;
import fd.functional.invoke;

#if __has_include("C_BaseEntity_generated_cpp")
#include "C_BaseEntity_generated_cpp"
#endif

using namespace fd;
using namespace valve;

data_map* base_entity::GetDataDescMap()
{
    return invoke_vfunc(&base_entity::GetDataDescMap, 15, this);
}

data_map* base_entity::GetPredictionDescMap()
{
    return invoke_vfunc(&base_entity::GetPredictionDescMap, 17, this);
}
