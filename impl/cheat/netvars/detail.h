#pragma once

#include <nlohmann/json.hpp>

#include <filesystem>
#include <sstream>

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

		template <typename T>
			requires(std::constructible_from<path, T>)
		lazy_fs_creator(T&& source) noexcept(std::is_rvalue_reference_v<decltype(source)>)
			: path(std::forward<T>(source))
		{
		}
	};

	class lazy_fs_remover final : public std::filesystem::path
	{
	public:
		~lazy_fs_remover( );

		template <typename T>
			requires(std::constructible_from<path, T>)
		lazy_fs_remover(T&& source, bool all) noexcept(std::is_rvalue_reference_v<decltype(source)>)
			: path(std::forward<T>(source)), all_(all)
		{
		}

	private:
		bool all_;
	};

	struct lazy_files_storage
	{
		std::vector<lazy_file_writer> write;
		std::vector<lazy_fs_creator> create;
		std::vector<lazy_fs_remover> remove;
	};

	using netvars_storage = nlohmann::basic_json<ordered_map_json, std::vector, std::string, bool, ptrdiff_t, size_t, float>;

	struct netvars_data
	{
		lazy_files_storage lazy;
		netvars_storage storage;
	};
}
