#pragma once

#include <fd/valve/data_map.h>
#include <fd/valve/recv_table.h>

#include <string>

namespace fd
{
// unable to use valve::***->name because we sometimes override it

std::string      extract_type_std_array(std::string_view type, size_t size);
std::string      extract_type_valve_vector(std::string_view type);
std::string_view extract_type_vec3(std::string_view name);
std::string_view extract_type_integer(std::string_view name);

std::string      extract_type(std::string_view name, valve::recv_prop* prop);
std::string_view extract_type(std::string_view name, valve::data_map_description* field);

// m_***
std::string_view extract_type_by_prefix(std::string_view name, valve::recv_prop* prop);
std::string_view extract_type_by_prefix(std::string_view name, valve::data_map_description* field);
} // namespace fd