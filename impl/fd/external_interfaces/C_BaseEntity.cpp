module;

#include <functional>

module fd.csgo.interfaces.C_BaseEntity;
import fd.address;
import fd.netvars;

using namespace fd;
using namespace csgo;

#if __has_include("C_BaseEntity_generated_cpp")
#include "C_BaseEntity_generated_cpp"
#endif

datamap_t* C_BaseEntity::GetDataDescMap()
{
    const basic_address vtable_holder                = this;
    const decltype(&C_BaseEntity::GetDataDescMap) fn = vtable_holder.deref<1>()[15];
    return std::invoke(fn, this);
}

datamap_t* C_BaseEntity::GetPredictionDescMap()
{
    const basic_address vtable_holder                      = this;
    const decltype(&C_BaseEntity::GetPredictionDescMap) fn = vtable_holder.deref<1>()[17];
    return std::invoke(fn, this);
}
