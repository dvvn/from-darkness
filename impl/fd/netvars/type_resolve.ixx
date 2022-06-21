module;

#include <string>
#include <variant>

export module fd.netvars.core:type_resolve;
export import fd.csgo.structs.Recv;
export import fd.csgo.structs.DataMap;
import fd.string_or_view;

export namespace fd::netvars
{
    std::string type_std_array(const std::string_view type, const size_t size);
    std::string type_utlvector(const std::string_view type);
    std::string_view type_vec3(const std::string_view type);
    std::string_view type_integer(std::string_view type);

    string_or_view type_recv_prop(const csgo::RecvProp* const prop);
    std::string_view type_datamap_field(const csgo::typedescription_t* const field);

    // m_***
    std::string_view type_array_prefix(const std::string_view type, csgo::RecvProp* const prop);
    std::string_view type_array_prefix(const std::string_view type, csgo::typedescription_t* const field);
} // namespace fd::netvars
