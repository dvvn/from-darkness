#pragma once

namespace nstd::detail
{
	template <typename T, typename ...Ts>
	FORCEINLINE constexpr T bitflag_combine(Ts ...args)
	{
		if constexpr (sizeof...(Ts) == 0)
		{
			return static_cast<T>(0);
		}
		else
		{
			if constexpr (!std::is_enum_v<T>)
				return (static_cast<T>(args) | ...);
			else
			{
				auto tmp = bitflag_combine<std::underlying_type_t<T>>(args...);
				return static_cast<T>(tmp);
			}
		}
	}

	template <typename T, typename T1>
	FORCEINLINE constexpr bool bitflag_has(T enum_val, T1 val)
	{
		if constexpr (!std::is_enum_v<T>)
			return enum_val & static_cast<T>(val);
		else
			return bitflag_has(static_cast<std::underlying_type_t<T>>(enum_val), val);
	}

	template <typename T, typename T1>
	FORCEINLINE constexpr T bitflag_remove(T enum_val, T1 val)
	{
		if constexpr (!std::is_enum_v<T>)
		{
			return enum_val & ~static_cast<T>(val);
		}
		else
		{
			auto tmp = bitflag_remove(static_cast<std::underlying_type_t<T>>(enum_val), val);
			return static_cast<T>(tmp);
		}
	}

	template <typename T>
	FORCEINLINE constexpr auto bitflag_raw_type( )
	{
		if constexpr (std::is_enum_v<T>)
			return std::underlying_type_t<T>( );
		else
			return T( );
	}
}

#define NSTD_ENUM_STRUCT(_NAME_, ...)\
	using value_type_raw = decltype(nstd::detail::bitflag_raw_type<value_type>());\
	\
	constexpr _NAME_(value_type_raw val = nstd::detail::bitflag_combine<value_type>(__VA_ARGS__)):\
		value_(nstd::detail::bitflag_combine<value_type>(val)){ static_assert(std::is_final_v<_NAME_>); }\
	constexpr auto operator<=>(const _NAME_&) const = default;\
	\
	constexpr value_type value( ) const { return value_; }\
	constexpr value_type_raw value_raw( ) const { return value_; }\
private:\
	value_type value_;

#define NSTD_ENUM_STRUCT_BITFLAG(_NAME_, ...)\
	using value_type_raw = decltype(nstd::detail::bitflag_raw_type<value_type>());\
	\
	constexpr auto operator<=>(const _NAME_&) const = default;\
	\
	template <typename ...T>\
		requires(std::convertible_to<T, value_type_raw> && ...)\
	constexpr _NAME_(T... vals): value_(nstd::detail::bitflag_combine<value_type>(vals..., ##__VA_ARGS__)) { static_assert(std::is_final_v<_NAME_>); }\
	template <typename ...T>\
		requires(std::convertible_to<T, value_type_raw> && ...)\
	constexpr bool has(T... vals) const { return (nstd::detail::bitflag_has(value_, vals) || ...); }\
	constexpr bool has(_NAME_ other) const { return this->has(other.value_); }\
	template <typename ...T>\
		requires(std::convertible_to<T, value_type_raw> && ...)\
	constexpr bool has_all(T... vals) const { return nstd::detail::bitflag_has(value_, nstd::detail::bitflag_combine<value_type>(vals...));}\
	template <typename ...T>\
		requires(std::convertible_to<T, value_type_raw> && ...)\
	constexpr _NAME_& add(T... vals) { value_ = nstd::detail::bitflag_combine<value_type>(value_, vals...); return *this; }\
	template <typename ...T>\
		requires(std::convertible_to<T, value_type_raw> && ...)\
	constexpr _NAME_& remove(T... vals) { value_ = nstd::detail::bitflag_remove<value_type>(value_, vals...); return *this; }\
	\
	constexpr value_type value() const {return value_;}\
	constexpr value_type_raw value_raw() const {return static_cast<value_type_raw>(value_);}\
private:\
	value_type value_;
