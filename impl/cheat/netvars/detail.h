#pragma once

#include "config.h"
#ifndef CHEAT_NETVARS_DUMPER_DISABLED
#include <nlohmann/json.hpp>

#include <filesystem>
#include <sstream>
#endif


namespace cheat::detail::netvars
{
#ifndef CHEAT_NETVARS_DUMPER_DISABLED
	template <class Key, class T, class IgnoredLess = std::less<Key>    //
			, class Allocator = std::allocator<std::pair<const Key, T>> //
			, class Base = nlohmann::ordered_map<Key, T, IgnoredLess, Allocator>>
	struct ordered_map_json : Base
	{
		using typename Base::key_type;
		using typename Base::mapped_type;

		using typename Base::Container;

		using typename Base::iterator;
		using typename Base::const_iterator;
		using typename Base::size_type;
		using typename Base::value_type;

		template <std::equality_comparable_with<Key> Key2, typename T1 = T>
			requires(std::constructible_from<Key, Key2> && std::constructible_from<T, T1>)
		std::pair<iterator, bool> emplace(Key2&& key, T1&& t = {})
		{
			auto end   = this->end( );
			auto found = this->find(key);
			if (found != end)
				return {found, false};

			Container::emplace_back(std::forward<Key2>(key), std::forward<T1>(t));
			return {std::prev(end), true};
		}

		template <std::equality_comparable_with<Key> Key2>
		iterator find(const Key2& key)
		{
			constexpr auto trivial = std::is_trivially_copyable_v<Key> && std::is_trivially_destructible_v<Key>;
			return std::ranges::find(*this, key, [](auto&& val)-> std::conditional_t<trivial, Key, const Key&> { return val.first; });
		}

		template <std::equality_comparable_with<Key> Key2>
		const_iterator find(const Key2& key) const
		{
			return std::_Const_cast(this)->find(key);
		}
	};

	class lazy_file_writer final : public std::ostringstream
	{
	public:
		~lazy_file_writer( ) override;

		lazy_file_writer(std::filesystem::path&& file);
		lazy_file_writer(lazy_file_writer&& other) noexcept;
		lazy_file_writer& operator=(lazy_file_writer&& other) noexcept;

	private:
		std::filesystem::path file_;
	};

	class lazy_fs_creator final : public std::filesystem::path
	{
	public:
		~lazy_fs_creator( );
		lazy_fs_creator(const path& path);
	};

	class lazy_fs_remover final : public std::filesystem::path
	{
	public:
		~lazy_fs_remover( );
		lazy_fs_remover(const path& path, bool all);

	private:
		bool all_;
	};

	struct lazy_files_storage
	{
		std::vector<lazy_file_writer> write;
		std::vector<lazy_fs_creator> create;
		std::vector<lazy_fs_remover> remove;
	};

	using netvars_storage = nlohmann::basic_json<ordered_map_json>;

#endif

	struct netvars_data
	{
#ifndef CHEAT_NETVARS_DUMPER_DISABLED
		lazy_files_storage lazy;
		netvars_storage storage;
#endif
	};
}
