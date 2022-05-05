module;

#include <nstd/type_traits.h>

#include <string_view>

export module cheat.tools.object_name;
export import nstd.type_name;

template<typename T>
consteval std::string_view drop_type_namespace(const std::string_view drop) noexcept
{
	using val_t = nstd::remove_all_pointers_t<T>;
	const auto str = nstd::type_name<val_t>( );
	return str.starts_with(drop) ? str.substr(drop.size( ) + 2) : str;
}

export namespace cheat::/*inline*/ tools
{
	template<typename T>
	constexpr std::string_view object_name = drop_type_namespace<T>("cheat");

	template<typename T>
	constexpr std::string_view csgo_object_name = drop_type_namespace<T>("cheat::csgo");
}