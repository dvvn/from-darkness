module;

#include <nstd/type_traits.h>

#include <string_view>
#ifdef _UNICODE
#include <array>
#include <algorithm>
#endif

export module cheat.tools.object_name;
export import nstd.type_name;

#ifdef _UNICODE
template<size_t Size>
class unicode_buffer
{
	std::array<wchar_t, Size + 1> buff_;

public:
	constexpr unicode_buffer(const char* ptr)
	{
		std::copy_n(ptr, Size, buff_.data( ));
		buff_.back( ) = 0;
	}

	constexpr operator std::wstring_view( ) const noexcept
	{
		return {buff_.data( ), Size};
	}

};
#endif

constexpr std::string_view drop_namespace_simple(const std::string_view str, const std::string_view drop) noexcept
{
	return str.starts_with(drop) ? str.substr(drop.size( ) + 2) : str;
}

export namespace cheat::inline tools
{
	template<typename T>
	constexpr std::string_view object_name( ) noexcept
	{
		return drop_namespace_simple(nstd::type_name<T>( ), "cheat");
	}

	template<typename T>
	constexpr std::string_view csgo_object_name( ) noexcept
	{
		if constexpr (std::is_pointer_v<T>)
			return csgo_object_name<nstd::remove_all_pointers_t<T>>( );
		else
			return drop_namespace_simple(nstd::type_name<T>( ), "cheat::csgo");
	}

	template<typename T>
	constexpr auto csgo_object_name_uni( ) noexcept
	{
		constexpr auto name = csgo_object_name<T>( );
#ifdef _UNICODE
		constexpr unicode_buffer<name.size( )> buff = name.data( );
		return buff;
#else
		return name
#endif
	}
}