#pragma once

#ifdef _DEBUG
#define TSL_DEBUG
#endif
#include <ordered map/include/tsl/ordered_map.h>
#include <ordered map/include/tsl/ordered_set.h>
#include <robin map/include/tsl/robin_map.h>
#include <robin map/include/tsl/robin_set.h>

#include <include/veque.hpp>

namespace nstd
{
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

#ifdef VEQUE_HEADER_GUARD
	template <typename T, typename A = std::allocator<T>>
	using deque = veque::veque<T, veque::fast_resize_traits, A>;
#define NSTD_HAVE_DEQUE
#endif

	namespace detail
	{
		template <typename T, typename A>
		using custom_deque =
#ifdef VEQUE_HEADER_GUARD
		deque
#else
			std::deque
#endif
		<T, A>;
	}

#ifdef TSL_ROBIN_MAP_H
	#define NSTD_HAVE_UNORDERED_MAP

	template <typename Key, typename Value,
			  typename Hasher = hash<Key>,
			  typename KeyEqual = equal_to<Key>,
			  typename Alloc = std::allocator<std::pair<Key, Value>>>
	using unordered_map = tsl::robin_map<Key, Value, Hasher, KeyEqual, Alloc>;
#endif

#ifdef TSL_ROBIN_SET_H
	#define NSTD_HAVE_UNORDERED_SET

	template <typename Value,
			  typename Hasher = hash<Value>,
			  typename KeyEqual = equal_to<Value>,
			  typename Alloc = std::allocator<Value>>
	using unordered_set = tsl::robin_set<Value, Hasher, KeyEqual, Alloc>;
#endif

#ifdef TSL_ORDERED_MAP_H
#define NSTD_HAVE_ORDERED_MAP

	template <typename Key, typename Value,
			  template<typename ValueUsed, typename Alloc>
			  typename Base = detail::custom_deque,
			  typename ValueUsed = std::pair<Key, Value>,
			  typename Alloc = std::allocator<ValueUsed>>
	using ordered_map = tsl::ordered_map<Key, Value, hash<Key>, equal_to<Key>, Alloc, Base<ValueUsed, Alloc>>;
#endif

#ifdef TSL_ORDERED_SET_H
#define NSTD_HAVE_ORDERED_SET

	template <typename Key, typename Value,
			  template<typename ValueUsed, typename Alloc>
			  typename Base = detail::custom_deque,
			  typename ValueUsed = std::pair<Key, Value>,
			  typename Alloc = std::allocator<ValueUsed>>
	using ordered_set = tsl::ordered_set<Key, Value, hash<Key>, equal_to<Key>, Alloc, Base<ValueUsed, Alloc>>;
#endif
}
