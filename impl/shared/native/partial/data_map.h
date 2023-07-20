#pragma once
#include "internal/native_interface.h"
#include "native/data_map.h"

namespace fd
{
template <class T>
union partial_data_map
{
    FD_NATIVE_INTERFACE_FN(T);
    function<15, native_data_map *> description;
    function<17, native_data_map *> prediction;
};
} // namespace fd