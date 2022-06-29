module;

#include <string>
#include <variant>

export module fd.netvars.core:type_resolve;
export import fd.valve.recv_table;
export import fd.valve.data_map;
import fd.string_or_view;

using namespace fd::valve;

export namespace fd::netvars
{
    std::string type_std_array(const std::string_view type, const size_t size);
    std::string type_utlvector(const std::string_view type);
    std::string_view type_vec3(const std::string_view type);
    std::string_view type_integer(std::string_view type);

    string_or_view type_recv_prop(const recv_prop* const prop);
    std::string_view type_datamap_field(const data_map_description* const field);

    // m_***
    std::string_view type_array_prefix(const std::string_view type, recv_prop* const prop);
    std::string_view type_array_prefix(const std::string_view type, data_map_description* const field);
} // namespace fd::netvars
