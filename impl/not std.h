#pragma once

#ifndef _STRINGIZE
#define NSTD_STRINGIZEX(x)  #x
#define NSTD_STRINGIZE(x)   NSTD_STRINGIZEX(x)
#else
#define NSTD_STRINGIZE _STRINGIZE
#endif

#define NSTD_RAW(x) NSTD_CONCAT(R,NSTD_STRINGIZE(##(x)##))

namespace nstd
{
	using ::thread_pool;

	template <class ...Fs>
	struct overload: Fs...
	{
		template <class ...Ts>
		overload(Ts&& ...ts) : Fs{std::forward<Ts>(ts)}...
		{
		}

		using Fs::operator()...;
	};

	template <class ...Ts>
	overload(Ts&&...) -> overload<std::remove_reference_t<Ts>...>;

	template <typename T>
	struct equal_to: std::equal_to<T>
	{
	};

	template <class Elem, class Traits>
	struct equal_to<std::basic_string_view<Elem, Traits>>
	{
		_NODISCARD constexpr bool operator()(const std::basic_string_view<Elem, Traits>& _Left,
											 const std::basic_string_view<Elem, Traits>& _Right) const
		{
			return _Left == _Right;
		}

		using is_transparent = void;
	};

	template <class Elem, class Traits, class Alloc>
	struct equal_to<std::basic_string<Elem, Traits, Alloc>>: equal_to<std::basic_string_view<Elem, Traits>>
	{
	};

	template <typename T>
	struct hash: std::hash<T>
	{
	};

	template <class Elem, class Traits, class Alloc>
	struct hash<std::basic_string<Elem, Traits, Alloc>>: std::hash<std::basic_string_view<Elem, Traits>>
	{
	};

	template <typename T, typename A = std::allocator<T>>
	using deque = veque::veque<T, veque::fast_resize_traits, A>;

#ifdef TSL_ROBIN_MAP_H

	template <typename Key, typename Value,
			  typename Hasher = hash<Key>,
			  typename KeyEqual = equal_to<Key>,
			  typename Alloc = std::allocator<std::pair<Key, Value>>>
	using _Unordered_map = tsl::robin_map<Key, Value, Hasher, KeyEqual, Alloc>;

	template <typename Key, typename Value>
	using unordered_map = _Unordered_map<Key, Value>;
#endif

#ifdef TSL_ROBIN_SET_H

	template <typename Value,
			  typename Hasher = hash<Value>,
			  typename KeyEqual = equal_to<Value>,
			  typename Alloc = std::allocator<Value>>
	using _Unordered_set = tsl::robin_set<Value, Hasher, KeyEqual, Alloc>;

	template <typename Value>
	using unordered_set = _Unordered_set<Value>;
#endif

#ifdef TSL_ORDERED_MAP_H
	template <typename Key, typename Value,
			  template<typename ValueUsed, typename Alloc> typename Base,
			  typename ValueUsed = std::pair<Key, Value>,
			  typename Alloc = std::allocator<ValueUsed>>
	using _Ordered_map = tsl::ordered_map<Key, Value, hash<Key>, equal_to<Key>, Alloc, Base<ValueUsed, Alloc>>;

	template <typename K, typename T>
	using ordered_map = _Ordered_map<K, T, deque>;

#endif

#ifdef TSL_ORDERED_SET_H

	template <typename Key, typename Value,
			  template<typename ValueUsed, typename Alloc> typename Base,
			  typename ValueUsed = std::pair<Key, Value>,
			  typename Alloc = std::allocator<ValueUsed>>
	using _Ordered_set = tsl::ordered_set<Key, Value, hash<Key>, equal_to<Key>, Alloc, Base<ValueUsed, Alloc>>;

	template <typename K, typename T>
	using ordered_set = _Ordered_set<K, T, deque>;

#endif
}

namespace nstd
{
	template <typename T>
	concept string_viewable = requires(const T& obj)
	{
		obj.view( );
	};

	namespace detail
	{
		struct checksum_impl
		{
			template <typename E, typename Tr>
			size_t operator ()(const std::basic_string_view<E, Tr>& str) const
			{
				return std::_Hash_array_representation(str._Unchecked_begin( ), str.size( ));
			}

			template <string_viewable T>
			size_t operator ()(const T& obj) const
			{
				return std::invoke(*this, obj.view( ));
			}

			template <typename T>
				requires(std::is_trivially_destructible_v<T>)
			size_t operator ()(const std::span<T>& vec) const
			{
				return (std::_Hash_array_representation(vec._Unchecked_begin( ), vec.size( )));
			}

			size_t operator()(const std::filesystem::path& p) const
			{
				if (exists(p))
				{
					auto ifs = std::ifstream(p);
					using itr_t = std::istreambuf_iterator<char>;
					if (!ifs.fail( ))
					{
						const auto tmp = std::vector(itr_t(ifs), itr_t( ));
						return std::invoke(*this, std::span(tmp));
					}
				}

				return 0;
			}
		};
	}

	inline constexpr auto checksum = detail::checksum_impl( );
}

namespace nstd
{
	template <typename T, typename Formatter = std::formatter<decltype(std::declval<T>( ).view( ))>>
	struct formatter_string_viewable: Formatter
	{
		template <class FormatContext>
		typename FormatContext::iterator format(const T& val, FormatContext& ctx)
		{
			return Formatter::format(val.view( ), ctx);
		}
	};
}

namespace std
{
	template <nstd::string_viewable T>
	struct formatter<T>: nstd::formatter_string_viewable<T>
	{
	};

	template <typename E, typename Tr, nstd::string_viewable T>
		requires(same_as<E, typename decltype(declval<T>( ).view( ))::value_type>)
	basic_ostream<E, Tr>& operator<<(basic_ostream<E, Tr>& s, const T& val)
	{
		return s << val.view( );
	}

	template <typename E, typename Tr, nstd::string_viewable T>
		requires(same_as<E, typename decltype(declval<T>( ).view( ))::value_type>)
	basic_ostream<E, Tr>&& operator<<(basic_ostream<E, Tr>&& s, const T& val)
	{
		return move(s << val);
	}

	template <typename E, typename Tr>
	basic_ostream<E, Tr>& operator<<(basic_ostream<E, Tr>& s, const nstd::chr_wchr& val)
	{
		switch (val.index( ))
		{
			case 0:
				return s << val.get<0>( );
			case 1:
				return s << val.get<1>( );
			default:
				throw;
		}
	}
}

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
			return enum_val & ~static_cast<T>(val);
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
