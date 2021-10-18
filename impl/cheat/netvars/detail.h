#pragma once

#include "config.h"
#ifndef CHEAT_NETVARS_DUMPER_DISABLED
#include <nlohmann/json.hpp>

#include <filesystem>
#include <sstream>
#endif

namespace cheat::detail
{
#ifndef CHEAT_NETVARS_DUMPER_DISABLED
	template <class Key, class T, class IgnoredLess = std::less<Key>,
			  class Allocator = std::allocator<std::pair<const Key, T>>>
	struct ordered_map_json : nlohmann::ordered_map<Key, T, IgnoredLess, Allocator>
	{
		using key_type = Key;
		using mapped_type = T;
		using Container = std::vector<std::pair<const Key, T>, Allocator>;
		using typename Container::iterator;
		using typename Container::const_iterator;
		using typename Container::size_type;
		using typename Container::value_type;

		std::pair<iterator, bool> emplace(key_type&& key, T&& t)
		{
			for (auto it = this->begin( ); it != this->end( ); ++it)
			{
				if (it->first == key)
				{
					return {it, false};
				}
			}
			Container::emplace_back(std::move(key), std::move(t));
			return {--this->end( ), true};
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
