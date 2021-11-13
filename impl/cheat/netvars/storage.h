#pragma once

#include <nlohmann/json.hpp>

#define NSTD_UTG

namespace cheat::detail::netvars
{
	template <class Key, class Value, class IgnoredLess = std::less<Key>    //
			, class Allocator = std::allocator<std::pair<const Key, Value>> //
			, class Base = nlohmann::ordered_map<Key, Value, IgnoredLess, Allocator>>
	struct ordered_map_json : Base
	{
		using typename Base::key_type;
		using typename Base::mapped_type;

		using typename Base::Container;

		using typename Base::iterator;
		using typename Base::const_iterator;
		using typename Base::size_type;
		using typename Base::value_type;

		template <std::equality_comparable_with<Key> Key2, typename Value1 = Value>
			requires(std::constructible_from<Key, Key2> && std::constructible_from<Value, Value1>)
		std::pair<iterator, bool> emplace(Key2&& key, Value1&& t = {})
		{
			auto found = this->find(key);
			if (found != this->end( ))
				return {found, false};

			this->emplace_back(std::forward<Key2>(key), std::forward<Value1>(t));
			return {std::prev(this->end( )), true};
		}

		template <std::equality_comparable_with<Key> Key2>
		iterator find(const Key2& key)
		{
			auto begin = this->begin( );
			auto end   = this->end( );
			for (auto itr = begin; itr != end; ++itr)
			{
				if (itr->first == key)
					return itr;
			}
			return end;
		}

		template <std::equality_comparable_with<Key> Key2>
		const_iterator find(const Key2& key) const
		{
			return std::_Const_cast(this)->find(key);
		}
	};

	struct netvars_root_storage : nlohmann::basic_json<ordered_map_json, std::vector, std::string, bool, ptrdiff_t, size_t, float>
	{
		template <std::derived_from<iterator> T>
		T erase(T pos)
			requires(std::constructible_from<T, iterator>)
		{
			return T(static_cast<value_type*>(this)->erase(static_cast<iterator&&>(pos)));
		}
		using nlohmann::basic_json<ordered_map_json, std::vector, std::string, bool, ptrdiff_t, size_t, float>::erase;
	};

	struct netvars_storage : netvars_root_storage
	{
	};
}
