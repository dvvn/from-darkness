module;

#include "storage_includes.h"

export module cheat.netvars:storage;

export namespace cheat::netvars_impl
{
#if 1
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

		template <std::equality_comparable_with<Key> Key2, typename ...Args>
			requires(std::constructible_from<Key, Key2>)
		std::pair<iterator, bool> emplace(Key2&& key, Args&&...args)
		{
			auto found = this->find(key);
			if (found != this->end( ))
				return {found, false};

			if constexpr (sizeof...(Args) == 0)
			{
				static_assert(std::default_initializable<Value>, "Unable to construct empty mapped type");
				this->emplace_back(std::forward<Key2>(key), Value( ));
			}
			else
			{
				this->emplace_back(std::forward<Key2>(key), std::forward<Args>(args)...);
			}
			return {std::prev(this->end( )), true};
		}

		template <std::equality_comparable_with<Key> Key2>
		iterator find(const Key2& key)
		{
			auto begin = this->begin( );
			auto end = this->end( );
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

		//----

		using _Char_type = Key::value_type;

		template<typename ...Args>
		std::pair<iterator, bool> emplace(const _Char_type* key, Args&&...) = delete;

		iterator find(const _Char_type* key) = delete;
		const_iterator find(const _Char_type* key) const = delete;
	};

	using netvars_storage = nlohmann::basic_json<ordered_map_json, std::vector, std::string, bool, ptrdiff_t, size_t, float>;

#else
	using netvars_storage = nlohmann::json;
#endif
}
