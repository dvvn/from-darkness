module;

export module fd.netvars.core:type_resolve;
export import fd.valve.recv_table;
export import fd.valve.data_map;
export import fd.string;

using namespace fd::valve;

export namespace fd::netvars
{
    string type_std_array(const string_view type, const size_t size);
    string type_utlvector(const string_view type);
    string_view type_vec3(const string_view type);
    string_view type_integer(string_view type);

    string type_recv_prop(const recv_prop* const prop);
    string_view type_datamap_field(const data_map_description* const field);

    // m_***
    string_view type_array_prefix(const string_view type, recv_prop* const prop);
    string_view type_array_prefix(const string_view type, data_map_description* const field);
} // namespace fd::netvars
