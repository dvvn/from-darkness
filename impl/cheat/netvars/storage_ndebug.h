#pragma once

#include <nstd/custom_types.h>
#include NSTD_UNORDERED_MAP_INCLUDE
#define NSTD_UTG NSTD_UNORDERED_TRANSPARENT_GAP

namespace cheat::detail::netvars
{
	using netvars_unordered_key =
#ifdef _DEBUG
	std::string
#else
size_t
#endif
	;

	using netvars_unordered_hash =
	NSTD_UNORDERED_HASH<
#ifdef _DEBUG
		std::string_view
#else
		size_t
#endif
	>;

	template <typename T>
	using netvars_unordered_map_t = NSTD_UNORDERED_MAP<netvars_unordered_key, T, netvars_unordered_hash, std::ranges::equal_to>;

	template <typename T>
	struct netvars_unordered_map : netvars_unordered_map_t<T>
	{
#ifndef _DEBUG

		T& operator[](const std::string_view& str)
		{
			auto hash = std::invoke(NSTD_UNORDERED_HASH<std::string_view>( ), str);
			return netvars_unordered_map_t<T>::operator[](hash);
		}

		T& at(const std::string_view& str)
		{
			auto hash = std::invoke(NSTD_UNORDERED_HASH<std::string_view>( ), str);
			return netvars_unordered_map_t<T>::at(hash);
		}

		const T& at(const std::string_view& str) const
		{
			return std::_Const_cast(this)->at(str);
		}

		decltype(auto) emplace(const std::string_view& str)
		{
			auto hash = std::invoke(NSTD_UNORDERED_HASH<std::string_view>( ), str);
			T dummy   = {};
			return netvars_unordered_map_t<T>::emplace(hash, std::move(dummy));
		}

#else

		template <class K>
			requires(std::constructible_from<std::string, K>)
		decltype(auto) emplace(K&& key)
		{
			T dummy = {};
			return netvars_unordered_map_t<T>::emplace((std::forward<K>(key)), std::move(dummy));
		}
#endif
	};

	struct netvars_storage : netvars_unordered_map<int>
	{
	};

	struct netvars_root_storage : netvars_unordered_map<netvars_storage>
	{
	};
}
