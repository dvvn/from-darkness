module;

#include <variant>

export module fd.netvars.core:type_resolve;
export import fd.valve.recv_table;
export import fd.valve.data_map;
export import fd.string_or_view;

using namespace fd::valve;

export namespace fd::netvars
{
    fd::string type_std_array(const fd::string_view type, const size_t size);
    fd::string type_utlvector(const fd::string_view type);
    fd::string_view type_vec3(const fd::string_view type);
    fd::string_view type_integer(fd::string_view type);

    string_or_view type_recv_prop(const recv_prop* const prop);
    fd::string_view type_datamap_field(const data_map_description* const field);

    // m_***
    fd::string_view type_array_prefix(const fd::string_view type, recv_prop* const prop);
    fd::string_view type_array_prefix(const fd::string_view type, data_map_description* const field);
} // namespace fd::netvars
