#pragma once
#include <type_traits>

namespace nstd
{
	template<typename T>
	requires(std::is_enum_v<T>)
	constexpr auto unwrap_enum(T val)
	{
		return (std::underlying_type_t<T>)(val);
	}

	template<std::integral T>
	constexpr auto unwrap_enum(T val)
	{
		return val;
	}
}
