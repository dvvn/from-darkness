module;

#include <nstd/runtime_assert.h>

#include <nlohmann/json.hpp>
#include <nlohmann/ordered_map.hpp>

#include <vector>
#include <sstream>
#include <fstream>
#include <filesystem>

module cheat.netvars.core:storage;
import cheat.console;
import nstd.mem.address;

namespace std::filesystem
{
	static_assert(sizeof(path) == sizeof(std::wstring));

	bool create_directory(const std::wstring& dir)
	{
		return create_directory(reinterpret_cast<const path&>(dir));
	}

	bool is_empty(const std::wstring& dir)
	{
		return is_empty(reinterpret_cast<const path&>(dir));
	}
}

using namespace cheat;
using namespace netvars;
namespace fs = std::filesystem;

template <typename T, typename... Args>
static T _Join_strings(const Args&... args)
{
	const auto string_size = (args.size() + ...);
	T out;
	out.reserve(string_size);
	(out.append(args.begin(), args.end()), ...);
	return out;
}

static bool _File_already_written(const fs::path& full_path, const std::string_view buffer)
{
	std::ifstream file_stored(full_path, std::ios::binary | std::ios::ate);
	if (!file_stored)
		return false;

	const auto size = static_cast<size_t>(file_stored.tellg());
	if (size != buffer.size())
		return false;

#if 0
	const auto buff = std::make_unique<char[]>(size);
	if (!file_stored.read(buff.get(), size))
		return false;
	return std::memcmp(buff.get(), buffer.data(), size) == 0;
#else
	using it_t = std::istream_iterator<char>;
	return std::equal<it_t>(file_stored, {}, buffer.begin());
#endif
}

logs_data::~logs_data()
{
	//moved
	if (dir.empty())
		return;

	if (!fs::create_directory(dir))
		return;

	const fs::path full_path = _Join_strings<std::wstring>(dir, file.name, file.extension);
	const auto new_file_data = buff.view();

	if (_File_already_written(full_path, new_file_data))
		return;

	std::ofstream(full_path) << new_file_data;
}

classes_data::~classes_data()
{
	//moved
	if (dir.empty())
		return;

	if (fs::create_directory(dir) || fs::is_empty(dir))
	{
		for (const auto& [name, buff] : files)
			std::ofstream(_Join_strings<std::wstring>(dir, name)) << buff.view();
		return;
	}

	//directory already exist

	for (const auto& [name, buff] : files)
	{
		const auto new_file_data = buff.view();
		const fs::path current_file_path = _Join_strings<std::wstring>(dir, name);
		if (_File_already_written(current_file_path, new_file_data))
			continue;

		std::ofstream(current_file_path) << new_file_data;
	}
}


template<typename Key, typename Key2>
concept equality_comparable_constructible = std::equality_comparable_with<Key, Key2> && std::constructible_from<Key, Key2>;

template <class Key>
struct ordered_map_json_key_proxy;

template <class Key, class Value, class IgnoredLess = std::less<Key>    //
, class Allocator = std::allocator<std::pair<const Key, Value>> //
, class Base = nlohmann::ordered_map<Key, Value, IgnoredLess, Allocator>>
	class ordered_map_json : public Base
{
	[[no_unique_address]]
	ordered_map_json_key_proxy<Key> proxy_;

public:
	using typename Base::key_type;
	using typename Base::mapped_type;

	using typename Base::Container;

	using typename Base::iterator;
	using typename Base::const_iterator;
	using typename Base::size_type;
	using typename Base::value_type;

	template <equality_comparable_constructible<Key> Key2, typename ...Args>
	std::pair<iterator, bool> emplace(Key2&& key, Args&&...args)
	{
		auto key_adapted = proxy_(std::forward<Key2>(key));

		auto found = this->find(key_adapted);
		if (found != Base::end())
			return { found, false };

		if constexpr (sizeof...(Args) == 0)
		{
			static_assert(std::default_initializable<Value>, "Unable to construct empty mapped type");
			Base::emplace_back(std::move(key_adapted), Value());
		}
		else
		{
			Base::emplace_back(std::move(key_adapted), std::forward<Args>(args)...);
		}
		return { std::prev(Base::end()), true };
    }

    template <std::equality_comparable_with<Key> Key2>
    iterator find(const Key2& key)
    {
		const auto key_adapted = proxy_(key);
		const auto begin = Base::begin();
		const auto end = Base::end();
		for (auto itr = begin; itr != end; ++itr)
		{
			if (itr->first == key_adapted)
				return itr;
		}
		return end;
    }

    template <std::equality_comparable_with<Key> Key2>
    const_iterator find(const Key2& key) const
    {
		return const_cast<ordered_map_json*>(this)->find(key);
	}
};

template<class Base>
struct json_wrapper : Base
{
#if NLOHMANN_JSON_VERSION_MAJOR < 3 || NLOHMANN_JSON_VERSION_MINOR < 11
	//string view because it anyway copy later
	auto& operator[](const std::string_view key)
	{
		return static_cast<Base*>(this)->operator[](Base::string_t(key.begin(), key.end()));
	}
#endif
};

template<>
struct ordered_map_json_key_proxy<std::string>
{
    std::string&& operator()(std::string&& str) const
    {
		return std::move(str);
    }

    std::string_view operator()(const std::string_view str) const
    {
		return str;
	}

	std::string_view operator()(const char* str) const
	{
		return str;
	}
};

void storage::log_netvars(logs_data& data)
{
	json_wrapper<nlohmann::basic_json<ordered_map_json, std::vector, std::string, bool, std::make_signed_t<size_t>, size_t, float>> j_root;

	for (const netvar_table& table : *this)
	{
		if (table.empty())
			continue;

		const std::string_view table_name = table.name();
		auto& j_table = j_root[table_name];

		for (const netvar_table::value_type& info : table)
		{
			const std::string_view name = info->name();
			const auto offset = info->offset();
			const std::string_view type = info->type();

			using namespace std::string_view_literals;
			j_table["name"] = name;
			j_table["offset"] = offset;
			if (!type.empty())
				j_table["type"] = type;
		}
	}

	data.buff << std::setw(data.indent) << std::setfill(data.filler) << j_root;
}

void storage::generate_classes(classes_data& data)
{
	data.files.reserve(this->size());

	for (const netvar_table& table : *this)
	{
		if (table.empty())
			continue;

		classes_data::file_info h_info, cpp_info;
		auto& h = h_info.buff;
		auto& cpp = cpp_info.buff;

		const std::string_view class_name = table.name();

		for (const netvar_table::value_type& info : table)
		{
			auto netvar_type = info->type();
			if (netvar_type.empty())
				continue;

			const auto type_pointer = netvar_type.ends_with('*');
			if (type_pointer)
				netvar_type.remove_suffix(1);
			const auto ret_char = type_pointer ? '*' : '&';

			const std::string_view netvar_name = info->name();

			//---

			const auto write_func_header = [=](std::basic_ostream<char>& stream, const bool inside_class)
			{
				stream << netvar_type << ret_char << ' ';
				if (!inside_class)
					stream << class_name << "::";
				stream << netvar_name << "( );\n";
			};

			write_func_header(h, true);

			cpp << '\n';
			write_func_header(cpp, false);
			cpp << "{\n"
				<< '	'
				<< "return netvars::apply_offset"
#ifndef CHEAT_NETVARS_LOG_STATIC_OFFSET
				<< "<\"" << class_name << "\">, <\"" << netvar_name << "\">"
#endif
				<< "(this"
#ifdef CHEAT_NETVARS_LOG_STATIC_OFFSET
				<< ", " << info.offset()
#endif
				<< ");\n}\n";
		}

		auto& h_name = h_info.name;
		auto& cpp_name = cpp_info.name;

		constexpr std::wstring_view h_postfix = L"_h";
		h_name.reserve(class_name.size() + h_postfix.size());
		constexpr std::wstring_view cpp_postfix = L"_cpp";
		cpp_name.reserve(class_name.size() + cpp_postfix.size());

		h_name = cpp_name = { class_name.begin(),class_name.end() };
		h_name += h_postfix;
		cpp_name += cpp_postfix;

		data.files.push_back(std::move(h_info));
		data.files.push_back(std::move(cpp_info));
	}
}
