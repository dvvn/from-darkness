#include "entity.h"

#include <fd/vfunc.h>

namespace fd::valve
{
data_map *get_desc_data_map(void *entity)
{
    return vtable(entity).call<data_map *>(15);
}

data_map *get_prediction_data_map(void *entity)
{
    return vtable(entity).call<data_map *>(17);
}
}