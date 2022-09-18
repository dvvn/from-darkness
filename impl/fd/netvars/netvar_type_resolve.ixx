module;

export module fd.netvars.type_resolve;
export import fd.valve.recv_table;
export import fd.valve.data_map;
export import fd.string;

export namespace fd::netvars
{
    string type_std_array(const string_view type, const size_t size);
    string type_utlvector(const string_view type);
    string_view type_vec3(const string_view type);
    string_view type_integer(string_view type);

    string type_recv_prop(const valve::recv_prop* prop);
    string_view type_datamap_field(const valve::data_map_description* field);

    // m_***
    string_view type_array_prefix(const string_view type, const valve::recv_prop* prop);
    string_view type_array_prefix(const string_view type, const valve::data_map_description* field);
} // namespace fd::netvars
