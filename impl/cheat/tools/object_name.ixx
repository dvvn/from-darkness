module;

#include <string_view>

export module cheat.tools.object_name;
export import nstd.type_name;

constexpr std::string_view drop_namespace_simple(const std::string_view str, const std::string_view drop)
{
	return str.starts_with(drop) ? str.substr(drop.size( ) + 2) : str;
}

export namespace cheat::inline tools
{
	template<typename T>
	constexpr std::string_view object_name( )
	{
		return drop_namespace_simple(nstd::type_name<T>( ), "cheat");
	}

	template<typename T>
	constexpr std::string_view csgo_object_name( )
	{
		return drop_namespace_simple(nstd::type_name<T>( ), "cheat::csgo");
	}
}