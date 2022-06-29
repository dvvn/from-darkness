module;

#include <functional>

module fd.valve.base_entity;
import fd.address;
import fd.netvars;

#if __has_include("C_BaseEntity_generated_cpp")
#include "C_BaseEntity_generated_cpp"
#endif

using namespace fd;
using namespace valve;

data_map* base_entity::GetDataDescMap()
{
    const basic_address vtable_holder               = this;
    const decltype(&base_entity::GetDataDescMap) fn = vtable_holder.deref<1>()[15];
    return std::invoke(fn, this);
}

data_map* base_entity::GetPredictionDescMap()
{
    const basic_address vtable_holder                     = this;
    const decltype(&base_entity::GetPredictionDescMap) fn = vtable_holder.deref<1>()[17];
    return std::invoke(fn, this);
}
