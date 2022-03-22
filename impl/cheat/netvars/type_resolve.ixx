module;

#include <string>
#include <variant>

export module cheat.netvars:type_resolve;
export import cheat.csgo.structs.Recv;
export import cheat.csgo.structs.DataMap;
import nstd.text.string_or_view;

using string_or_view = nstd::text::string_or_view_holder;

export namespace cheat::netvars
{
	std::string type_std_array(const std::string_view type,const size_t size);
	std::string type_utlvector(const std::string_view type);
	std::string_view type_vec3(const std::string_view type);
	std::string_view type_integer(std::string_view type);

	string_or_view type_recv_prop(const csgo::RecvProp* prop);
	std::string_view type_datamap_field(const csgo::typedescription_t* field);

	//m_***
	std::string_view type_array_prefix(const std::string_view type, const csgo::RecvProp* prop);
	std::string_view type_array_prefix(const std::string_view type, const csgo::typedescription_t* field);
}
