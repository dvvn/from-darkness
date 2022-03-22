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

module cheat.netvars:storage;
import cheat.console;
import nstd.mem.address;

using namespace cheat;
using namespace netvars;
namespace fs = std::filesystem;

static_assert(sizeof(fs::path) == sizeof(std::wstring));

template<typename T, typename ...Args>
static T join_strings(const Args& ...args)
{
	T out;
	out.reserve((args.size( ) + ...));
	(out.append(args.begin( ), args.end( )), ...);
	return out;
}

static bool file_already_written(const fs::path& full_path, const std::string_view buffer)
{
	auto file_stored = std::ifstream(full_path, std::ios::binary | std::ios::ate);
	if (!file_stored)
		return false;

	const auto size = static_cast<size_t>(file_stored.tellg( ));
	if (size != buffer.size( ))
		return false;

	auto buff = std::make_unique<char[]>(size);
	if (!file_stored.read(buff.get( ), size))
		return false;

	return std::memcmp(buff.get( ), buffer.data( ), size) == 0;
}

logs_data::~logs_data( )
{
	//moved
	if (dir.empty( ))
		return;

	const fs::path full_path = join_strings<std::wstring>(dir, file.name, file.extension);
	const auto new_file_data = buff.view( );

	if (!fs::create_directory(reinterpret_cast<fs::path&>(dir)) && file_already_written(full_path, new_file_data))
		return;

	std::ofstream(full_path) << new_file_data;
}

classes_data::~classes_data( )
{
	//moved
	if (dir.empty( ))
		return;

	if (fs::create_directory(reinterpret_cast<fs::path&>(dir)) || fs::is_empty(reinterpret_cast<fs::path&>(dir)))
	{
		for (const auto& [name, buff] : files)
			std::ofstream(join_strings<std::wstring>(dir, name)) << buff.view( );
		return;
	}

	//directory already exist

	for (const auto& [name, buff] : files)
	{
		const auto new_file_data = buff.view( );
		const fs::path current_file_path = join_strings<std::wstring>(dir, name);
		if (file_already_written(current_file_path, new_file_data))
			continue;

		std::ofstream(current_file_path) << new_file_data;
	}
}

struct json_string :std::string
{
	template<typename ...Args>
		requires(std::constructible_from<std::string, Args...>)
	json_string(Args&&...args)
		:std::string(std::forward<Args>(args)...)
	{
	}
};

void storage::log_netvars(logs_data& data)
{
	//single-file mode
	using json_type = nlohmann::basic_json<nlohmann::ordered_map, std::vector, json_string, bool, std::make_signed_t<size_t>, size_t, float>;
	json_type json;

	for (const netvar_table& table : *this)
	{
		if (table.empty( ))
			continue;

		auto& entry = json[table.name( )];
		for (const basic_netvar_info& info : table)
		{
			const auto name = info.name( );
			const auto offset = info.offset( );
			const auto type = info.type( );

			entry["name"] = name;
			entry["offset"] = offset;
			if (!type.empty( ))
				entry["type"] = type;
		}
	}

	data.buff << std::setw(data.indent) << std::setfill(data.filler) << std::move(json);
}

void storage::generate_classes(classes_data& data)
{
	data.files.reserve(this->size( ));

	for (const netvar_table& table : *this)
	{
		if (table.empty( ))
			continue;

		classes_data::file_info h_info, cpp_info;
		auto& h = h_info.buff;
		auto& cpp = cpp_info.buff;

		const std::string_view class_name = table.name( );

		for (const basic_netvar_info& info : table)
		{
			auto netvar_type = info.type( );
			if (netvar_type.empty( ))
				continue;

			const auto type_pointer = netvar_type.ends_with('*');
			if (type_pointer)
				netvar_type.remove_suffix(1);
			const auto ret_char = type_pointer ? '*' : '&';

			const std::string_view netvar_name = info.name( );

			//---

			const auto write_func_header = [=](std::basic_ostream<char>& stream, const bool inside_class)
			{
				stream << netvar_type << ret_char << ' ';
				if (!inside_class)
					stream << class_name << "::";
				stream << netvar_name << "( );\n";
			};

			write_func_header(h, true);

			write_func_header(cpp, false);
			cpp << "{\n"
				<< '	'
#ifdef CHEAT_NETVARS_LOG_STATIC_OFFSET
				<< "constexpr auto offset = " << info.offset( )
#else
				<< "static const auto offset = netvars::get_offset" << "(\"" << class_name << "\", \"" << netvar_name << "\")"
#endif
				<< ";\n"
				<< '	'
				<< "return " << nstd::type_name<nstd::mem::basic_address>( ) << "(this) + offset;\n"
				<< "}\n\n";
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