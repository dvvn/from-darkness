#pragma once

#include <fd/valve/data_map.h>
#include <fd/valve/recv_table.h>

#include <variant>

namespace fd
{
using netvar_source = std::variant< //
    std::monostate,
    valve::data_map_description*,
    valve::recv_prop*>;
}