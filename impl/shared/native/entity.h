#pragma once
#include "data_map.h"
#include ".detail/interface.h"

namespace fd
{
FD_BIND_NATIVE_INTERFACE(C_BaseEntity, client);

union native_entity
{
    FD_NATIVE_INTERFACE(C_BaseEntity);
    function<15, native_data_map *> get_desc_data_map;
    function<17, native_data_map *> get_prediction_data_map;
};
}