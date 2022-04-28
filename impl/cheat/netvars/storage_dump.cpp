module;

#include <nstd/runtime_assert.h>

#include <nlohmann/json.hpp>
#include <nlohmann/ordered_map.hpp>

//#include <nstd/format.h>
//#include <nstd/ranges.h>

#include <vector>
#include <sstream>
#include <fstream>
#include <filesystem>

module cheat.netvars.core:storage;
import cheat.console;
import nstd.mem.address;

using namespace cheat;
using namespace netvars;
namespace fs = std::filesystem;

static_assert(sizeof(fs::path) == sizeof(std::wstring));

template<typename T, typename ...Args>
static T _Join_strings(const Args& ...args) noexcept
{
	const auto string_size = (args.size( ) + ...);
	T out;
	out.reserve(string_size);
	(out.append(args.begin( ), args.end( )), ...);
	return out;
}

static bool _File_already_written(const fs::path& full_path, const std::string_view buffer) noexcept
{
	std::ifstream file_stored(full_path, std::ios::binary | std::ios::ate);
	if(!file_stored)
		return false;

	const auto size = static_cast<size_t>(file_stored.tellg( ));
	if(size != buffer.size( ))
		return false;

#if 0
	const auto buff = std::make_unique<char[ ]>(size);
	if(!file_stored.read(buff.get( ), size))
		return false;
	return std::memcmp(buff.get( ), buffer.data( ), size) == 0;
#else
	using it_t = std::istream_iterator<char>;
	return std::equal<it_t>(file_stored, {}, buffer.begin( ));
#endif
}

logs_data::~logs_data( )
{
	//moved
	if(dir.empty( ))
		return;

	if(!fs::create_directory(reinterpret_cast<fs::path&>(dir)))
		return;

	const fs::path full_path = _Join_strings<std::wstring>(dir, file.name, file.extension);
	const auto new_file_data = buff.view( );

	if(_File_already_written(full_path, new_file_data))
		return;

	std::ofstream(full_path) << new_file_data;
}

classes_data::~classes_data( )
{
	//moved
	if(dir.empty( ))
		return;

	if(fs::create_directory(reinterpret_cast<fs::path&>(dir)) || fs::is_empty(reinterpret_cast<fs::path&>(dir)))
	{
		for(const auto& [name, buff] : files)
			std::ofstream(_Join_strings<std::wstring>(dir, name)) << buff.view( );
		return;
	}

	//directory already exist

	for(const auto& [name, buff] : files)
	{
		const auto new_file_data = buff.view( );
		const fs::path current_file_path = _Join_strings<std::wstring>(dir, name);
		if(_File_already_written(current_file_path, new_file_data))
			continue;

		std::ofstream(current_file_path) << new_file_data;
	}
}

struct json_string :std::string
{
	template<typename ...Args>
	json_string(Args&&...args) requires(std::constructible_from<std::string, decltype(args)...>)
		: std::string(std::forward<Args>(args)...)
	{
	}
};

template<class Json>
static void to_json(Json& j, const basic_netvar_info& info) noexcept
{
	const std::string_view name = info.name( );
	const auto offset = info.offset( );
	const std::string_view type = info.type( );

	j["name"] = name;
	j["offset"] = offset;
	if(!type.empty( ))
		j["type"] = type;
}

template<class Json>
static void to_json(Json& j, const netvar_table& table) noexcept
{
	for(const netvar_table::value_type& info : table)
		j.emplace_back(*info);
}

//pointer to be 100% sure that this function is called instead of internal one
template<class Json>
static void to_json(Json& j, storage* const s) noexcept
{
	for(const netvar_table& table : *s)
	{
		if(table.empty( ))
			continue;

		j[table.name( )] = table;
	}
}

void storage::log_netvars(logs_data& data) noexcept
{
	//single-file mode

	using json_type = nlohmann::basic_json<nlohmann::ordered_map, std::vector, json_string, bool, std::make_signed_t<size_t>, size_t, float>;
	data.buff << std::setw(data.indent) << std::setfill(data.filler) << json_type(this);
}

void storage::generate_classes(classes_data& data) noexcept
{
	data.files.reserve(this->size( ));

	for(const netvar_table& table : *this)
	{
		if(table.empty( ))
			continue;

		classes_data::file_info h_info, cpp_info;
		auto& h = h_info.buff;
		auto& cpp = cpp_info.buff;

		const std::string_view class_name = table.name( );

		for(const netvar_table::value_type& info : table)
		{
			auto netvar_type = info->type( );
			if(netvar_type.empty( ))
				continue;

			const auto type_pointer = netvar_type.ends_with('*');
			if(type_pointer)
				netvar_type.remove_suffix(1);
			const auto ret_char = type_pointer ? '*' : '&';

			const std::string_view netvar_name = info->name( );

			//---

			const auto write_func_header = [=](std::basic_ostream<char>& stream, const bool inside_class)
			{
				stream << netvar_type << ret_char << ' ';
				if(!inside_class)
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
				<< ", " << info.offset( )
#endif
				<< ");\n}\n";
		}

		auto& h_name = h_info.name;
		auto& cpp_name = cpp_info.name;

		constexpr std::wstring_view h_postfix = L"_h";
		h_name.reserve(class_name.size( ) + h_postfix.size( ));
		constexpr std::wstring_view cpp_postfix = L"_cpp";
		cpp_name.reserve(class_name.size( ) + cpp_postfix.size( ));

		h_name = cpp_name = {class_name.begin( ),class_name.end( )};
		h_name += h_postfix;
		cpp_name += cpp_postfix;

		data.files.push_back(std::move(h_info));
		data.files.push_back(std::move(cpp_info));
	}
}