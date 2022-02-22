module;

#include <string>
#include <variant>

export module cheat.netvars:type_resolve;
export import cheat.csgo.interfaces;

export namespace cheat::netvars_impl
{
	struct string_or_view_holder
	{
		string_or_view_holder( );
		string_or_view_holder(const std::string_view& sv);
		string_or_view_holder(std::string&& str);

		string_or_view_holder(std::string& str) = delete;
		string_or_view_holder(const char*) = delete;

		std::string& str( )&;
		std::string str( )&&;
		std::string_view view( )const&;
		std::string_view view( )const&& = delete;

	private:
		std::variant<std::string, std::string_view> str_;
	};

	string_or_view_holder type_std_array(const std::string_view& type, size_t size);
	string_or_view_holder type_utlvector(const std::string_view& type);
	string_or_view_holder type_vec3(const std::string_view& name);
	string_or_view_holder type_integer(std::string_view name);

	string_or_view_holder type_recv_prop(const csgo::RecvProp& prop);
	string_or_view_holder type_datamap_field(const csgo::typedescription_t& field);
}
