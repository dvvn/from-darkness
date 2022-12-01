#pragma once

#include <fd/string.h>
#include <fd/valve/data_map.h>
#include <fd/valve/recv_table.h>

namespace fd
{
    string type_std_array(string_view type, size_t size);
    string type_utlvector(string_view type);
    string_view type_vec3(string_view type);
    string_view type_integer(string_view type);

    string type_recv_prop(const valve::recv_prop* prop);
    string_view type_datamap_field(const valve::data_map_description* field);

    // m_***
    string_view type_array_prefix(string_view type, const valve::recv_prop* prop);
    string_view type_array_prefix(string_view type, const valve::data_map_description* field);
} // namespace fd
