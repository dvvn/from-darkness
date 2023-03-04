#pragma once

#include <fd/netvars/type.h>

#include <fd/valve/data_map.h>
#include <fd/valve/recv_table.h>

namespace fd
{
struct netvar_type_hint
{
    uint16_t         arraySize;
    netvar_source    source;
    std::string_view name;

    netvar_type resolve() const;
};
} // namespace fd