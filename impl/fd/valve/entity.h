#pragma once
#include "data_map.h"

#include <fd/abstract_interface.h>

namespace fd::valve
{
// ReSharper disable once CppInconsistentNaming
class C_BaseEntity;

union entity
{
    FD_ABSTRACT_INTERFACE(C_BaseEntity);
    abstract_function<15, data_map *> get_desc_data_map;
    abstract_function<17, data_map *> get_prediction_data_map;
};
}