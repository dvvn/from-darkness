#pragma once
#include <type_traits>

template <typename T>
	requires(std::is_enum_v<T>)
constexpr T operator|(T a, T b)
{
	using raw_t = std::underlying_type_t<T>;
	raw_t ret = (raw_t)a | raw_t(b);
	return (T)(ret);
}

template <typename T>
	requires(std::is_enum_v<T>)
constexpr auto operator&(T a, T b)
{
	using raw_t = std::underlying_type_t<T>;
	raw_t ret = (raw_t)a & raw_t(b);
	return (ret);
}

template <typename T>
	requires(std::is_enum_v<T>)
constexpr T& operator|=(T& a, T b)
{
	return a = (a | b);
}

template <typename T>
	requires(std::is_enum_v<T>)
constexpr T& operator&=(T& a, T b)
{
	return a = (T)(a & b);
}

template <typename T>
	requires(std::is_enum_v<T>)
constexpr T operator~(T a)
{
	using raw_t = std::underlying_type_t<T>;
	raw_t ret = ~(raw_t)a;
	return a = (T)(ret);
}
