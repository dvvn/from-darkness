#pragma once

#include <nstd/custom_types.h>
#include NSTD_UNORDERED_MAP_INCLUDE
#ifdef _DEBUG
#define NSTD_UTG NSTD_UNORDERED_TRANSPARENT_GAP
#else
#define NSTD_UTG
#endif

namespace cheat::detail::netvars
{
	template <typename T>
	using netvars_unordered_map_t = NSTD_UNORDERED_MAP<
#ifdef _DEBUG
		std::string, T, NSTD_UNORDERED_HASH<std::string_view>, std::ranges::equal_to
#else
		size_t, T
#endif
	>;

	template <typename T, typename Base = netvars_unordered_map_t<T>>
	struct netvars_unordered_map : Base
	{
#ifndef _DEBUG

		T& operator[](const std::string_view& str)
		{
			auto hash = std::invoke(NSTD_UNORDERED_HASH<std::string_view>( ), str);
			return Base::operator[](hash);
		}

		T& at(const std::string_view& str)
		{
			auto hash = std::invoke(NSTD_UNORDERED_HASH<std::string_view>( ), str);
			return Base::at(hash);
		}

		const T& at(const std::string_view& str) const
		{
			return std::_Const_cast(this)->at(str);
		}

		typename Base::iterator find(const std::string_view& str)
		{
			auto hash = std::invoke(NSTD_UNORDERED_HASH<std::string_view>( ), str);
			return Base::find(hash);
		}

		typename Base::const_iterator find(const std::string_view& str) const
		{
			return std::_Const_cast(this)->find(str);
		}

		template <typename T1 = T>
		decltype(auto) emplace(const std::string_view& str, T1&& value = {})
		{
			auto hash = std::invoke(NSTD_UNORDERED_HASH<std::string_view>( ), str);
			return Base::emplace(hash, std::forward<T1>(value));
		}

#else

		template <class K>
		requires(std::constructible_from<std::string, K>)
			decltype(auto) emplace(K&& key)
		{
			T dummy = {};
			return Base::emplace((std::forward<K>(key)), std::move(dummy));
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
