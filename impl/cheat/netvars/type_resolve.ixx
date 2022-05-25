module;

#include <string>
#include <variant>

export module cheat.netvars.core:type_resolve;
export import cheat.csgo.structs.Recv;
export import cheat.csgo.structs.DataMap;
import nstd.text.string_or_view;

using string_or_view = nstd::text::string_or_view;

export namespace cheat::netvars
{
    std::string type_std_array(const std::string_view type, const size_t size);
    std::string type_utlvector(const std::string_view type);
    std::string_view type_vec3(const std::string_view type);
    std::string_view type_integer(std::string_view type);

    string_or_view type_recv_prop(csgo::RecvProp* const prop);
    std::string_view type_datamap_field(csgo::typedescription_t* const field);

    // m_***
    std::string_view type_array_prefix(const std::string_view type, csgo::RecvProp* const prop);
    std::string_view type_array_prefix(const std::string_view type, csgo::typedescription_t* const field);
}
