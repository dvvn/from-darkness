#pragma once
#include "data_map.h"

namespace fd::valve
{
data_map *get_desc_data_map(void *entity);
data_map *get_prediction_data_map(void *entity);
}