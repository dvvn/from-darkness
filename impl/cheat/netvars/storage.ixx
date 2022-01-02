module;

#include "storage_includes.h"

export module cheat.netvars:storage;

export namespace cheat::netvars_impl
{
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
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

		template <std::equality_comparable_with<Key> Key2, typename Value1 = Value>
			requires(std::constructible_from<Key, Key2>&& std::constructible_from<Value, Value1>)
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
	};

	using netvars_root_storage_base = nlohmann::basic_json<ordered_map_json, std::vector, std::string, bool, ptrdiff_t, size_t, float>;

#else
	using netvars_root_storage_base = nlohmann::json;
#endif

	struct netvars_root_storage : netvars_root_storage_base
	{
		using netvars_root_storage_base::iterator;

		template <std::derived_from<iterator> T>
		T erase(T pos)
			requires(std::constructible_from<T, iterator>)
		{
			return T(static_cast<value_type*>(this)->erase(static_cast<iterator&&>(pos)));
		}
		using netvars_root_storage_base::erase;
	};

	struct netvars_storage : netvars_root_storage
	{
	};

#else

	template <typename T>
	using netvars_unordered_map_t = nstd::unordered_map
		<
#ifdef _DEBUG
		std::string, T, nstd::hash<std::string_view>, std::ranges::equal_to
#else
		size_t, T
#endif
		>;

	template <typename T, typename Base = netvars_unordered_map_t<T>>
	struct netvars_unordered_map : Base
	{
#ifdef _DEBUG

		template <typename S>
		static auto string_hash(const S& str)
		{
			return std::invoke(nstd::hash<S>( ), str);
		}

		T& operator[](const std::string_view& str) { return Base::operator[](string_hash(str)); }

		T& at(const std::string_view& str) { return Base::at(string_hash(str)); }
		const T& at(const std::string_view& str) const { return std::_Const_cast(this)->at(str); }

		typename Base::iterator find(const std::string_view& str) { return Base::find(string_hash(str)); }
		typename Base::const_iterator find(const std::string_view& str) const { return std::_Const_cast(this)->find(str); }

#endif

		template <class K, typename T1 = T>
			requires(std::constructible_from<std::string, K>)
		decltype(auto) emplace(K&& key, T1&& value = {})
		{
			return Base::emplace(
#ifndef _DEBUG
				string_hash(key)
#else
				std::forward<K>(key)
#endif
				, std::forward<T1>(value));
		}
	};

	struct netvars_storage : netvars_unordered_map<int>
	{
	};

	struct netvars_root_storage : netvars_unordered_map<netvars_storage>
	{
	};

#endif

	template <typename T>
	struct iterator_wrapper : T
	{
		using iter_type = T;

		iterator_wrapper( ) = default;

		template <typename ...Args>
		iterator_wrapper(Args&&...args)
			: T(std::forward<Args>(args)...)
		{
		}

#ifndef CHEAT_NETVARS_RESOLVE_TYPE
		auto& operator*( ) { return T::operator->( )->second; }
		auto& operator*( ) const { return T::operator->( )->second; }
		auto operator->( ) { return std::addressof(T::operator->( )->second); }
		auto operator->( ) const { return std::addressof(T::operator->( )->second); }
#endif
	};

	template <typename T>
	iterator_wrapper(T&&)->iterator_wrapper<std::remove_cvref_t<T>>;

}
