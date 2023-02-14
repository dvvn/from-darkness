#pragma once

#include <fd/string.h>
#include <fd/valve/data_map.h>
#include <fd/valve/recv_table.h>

namespace fd
{
// unable to use valve::***->name because we sometimes override it

string      extract_type_std_array(string_view type, size_t size);
string      extract_type_valve_vector(string_view type);
string_view extract_type_vec3(string_view name);
string_view extract_type_integer(string_view name);

string      extract_type(string_view name, const valve::recv_prop* prop);
string_view extract_type(string_view name, const valve::data_map_description* field);

// m_***
string_view extract_type_by_prefix(string_view name, const valve::recv_prop* prop);
string_view extract_type_by_prefix(string_view name, const valve::data_map_description* field);
} // namespace fd