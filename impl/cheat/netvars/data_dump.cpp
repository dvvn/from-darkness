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

#if 0

#define CHEAT_NETVARS_GENERATED_DIR NSTD_STRINGIZE_RAW_WIDE(NSTD_CONCAT(VS_SolutionDir, \impl\cheat\csgo\interfaces_custom\))
#define CHEAT_CSGOSDK_DIR _CONCAT(VS_SolutionDir, \impl\cheat\csgo\)
#define CHEAT_NETVARS_GENERATED_HEADER_POSTFIX _h
#define CHEAT_NETVARS_GENERATED_SOURCE_POSTFIX _cpp
#define CHEAT_NETVARS_GENERATED_TAG _generated

#define STRINGIZE_PATH(_PATH_) _CONCAT(LR,_STRINGIZE(##(_PATH_)##))

template<typename T>
static decltype(auto) _To_wide(const T& str)
{
	using val_t = std::ranges::range_value_t<T>;

	if constexpr (!std::is_class_v<T>)
		return _To_wide(std::basic_string_view<val_t>(str));
	else if constexpr (std::same_as<val_t, wchar_t>)
		return str;
	else
		return str | std::views::transform(nstd::text::cast_all<wchar_t>);
}

template<typename ...Args>
static constexpr auto _To_wstring(const Args& ...args)
{
	return nstd::container::append<std::wstring>(_To_wide(args)...);
}

static auto _Correct_game_version_string(const std::string_view game_version)
{
	constexpr auto replace_dots = [](char c)
	{
		return c == '.' ? L'_' : static_cast<wchar_t>(c);
	};

	return game_version | std::views::transform(replace_dots);
}

bool netvars::log_netvars(const char* game_version, const storage& root_netvars_data, const log_file_config& cfg)
{
	[[maybe_unused]]
	const auto dirs_created = create_directories(cfg.dir);

	const auto netvars_dump_file = cfg.dir / _To_wstring(_Correct_game_version_string(game_version), cfg.extension);

	const auto file_exists = !dirs_created && exists(netvars_dump_file);
	if (!file_exists)
	{
		std::ofstream(netvars_dump_file) << std::setw(cfg.indent) << std::setfill(cfg.filler) << root_netvars_data;
		console::log("Netvars dump done");
		return true;
	}

	//------

	std::ostringstream netvars_data_as_text;
	netvars_data_as_text << std::setw(cfg.indent) << std::setfill(cfg.filler) << root_netvars_data;

	if (nstd::checksum(netvars_dump_file) != nstd::checksum(netvars_data_as_text))
	{
		std::ofstream(netvars_dump_file) << netvars_data_as_text.view( );
		console::log("Netvars dump updated");
		return true;
	}

	console::log("Netvars dump skipped");
	return false;
}

struct generated_file_data
{
	fs::path h_cached;
	fs::path cpp_cached;
	const netvars::storage::value_type* netvars;
};

void netvars::generate_classes(bool recreate, storage& root_netvars_data, lazy::files_storage& lazy_storage)
{
	const fs::path generated_classes_dir = STRINGIZE_PATH(CHEAT_NETVARS_GENERATED_DIR);

	constexpr std::string_view suffix_h = _STRINGIZE(CHEAT_NETVARS_GENERATED_HEADER_POSTFIX);
	constexpr std::wstring_view wsuffix_h = _CRT_WIDE(_STRINGIZE(CHEAT_NETVARS_GENERATED_HEADER_POSTFIX));

	constexpr std::string_view suffix_cpp = _STRINGIZE(CHEAT_NETVARS_GENERATED_SOURCE_POSTFIX);
	constexpr std::wstring_view wsuffix_cpp = _CRT_WIDE(_STRINGIZE(CHEAT_NETVARS_GENERATED_SOURCE_POSTFIX));

#ifdef CHEAT_NETVARS_GENERATED_TAG
	constexpr std::string_view generated_tag = _STRINGIZE(CHEAT_NETVARS_GENERATED_TAG);
	constexpr std::wstring_view wgenerated_tag = _CRT_WIDE(_STRINGIZE(CHEAT_NETVARS_GENERATED_TAG));
#endif

	//nstd::mem::backup<storage> netvars_data_backup;

	//files to be created from netvars classes
	nstd::unordered_map<std::string, generated_file_data> files_to_generate;
	for (const auto& [CLASS_NAME, NETVARS] : root_netvars_data.items( ))
	{
		generated_file_data& data = files_to_generate[CLASS_NAME];
		data.netvars = std::addressof(NETVARS);
	}

	const bool create_dir = !fs::exists(generated_classes_dir);
	size_t empty_files_count = root_netvars_data.size( ) * 2;

	if (create_dir)
	{
		recreate = false;
		lazy_storage.create.emplace_back(generated_classes_dir);
	}
	else if (!fs::is_empty(generated_classes_dir))
	{
		size_t not_generated = empty_files_count;

		auto not_generated_max = not_generated;
		for (const auto& file : fs::directory_iterator(generated_classes_dir))
		{
			if (!is_regular_file(file.path( )))
				continue;

			const auto wcfile_name = file.path( ).filename( ).native( );
			const bool h = wcfile_name.ends_with(wsuffix_h);
			const bool cpp = !h && wcfile_name.ends_with(wsuffix_cpp);

			std::wstring_view wfile_name = wcfile_name;
			if (h)
				wfile_name.remove_suffix(wsuffix_h.size( ));
			else if (cpp)
				wfile_name.remove_suffix(wsuffix_cpp.size( ));
			else
				continue;
#ifdef CHEAT_NETVARS_GENERATED_TAG
			if (wcfile_name.ends_with(wgenerated_tag))
				wfile_name.remove_suffix(wgenerated_tag.size( ));
			else
				continue;
#endif
			nstd::basic_unistring<char> file_name = wfile_name;

			auto data = files_to_generate.find(file_name);
			if (data == files_to_generate.end( ))
				continue;

			--not_generated;
			auto& cached = h ? data->second.h_cached : data->second.cpp_cached;
			cached = file.path( );
		}

		const auto run_fn = [&]<typename Fn>(Fn fn)
		{
			for (auto& [class_name, data] : files_to_generate)
			{
				std::invoke(fn, data.h_cached, class_name, suffix_h);
				std::invoke(fn, data.cpp_cached, class_name, suffix_cpp);
			}
		};

		const auto construct = [&]<typename ...Args>(const Args& ...args)
		{
			return generated_classes_dir / _To_wstring(args...);
		};

		//no files generated before
		if (not_generated_max == not_generated)
		{
			run_fn([&]<typename ...Args>(fs::path & p, const Args& ...args)
			{
				p = construct(args...);
			});
		}
		else if (recreate)
		{
			run_fn([&]<typename ...Args>(fs::path & p, const Args& ...args)
			{
				if (!p.empty( ))
					lazy_storage.remove.emplace_back(p, false);
				else
					p = construct(args...);
			});
		}
		else
		{
			if (not_generated == 0)
			{
				console::log("Netvars classes are up-to-date.");
				return;
			}

			empty_files_count = not_generated;

			run_fn([&]<typename ...Args>(fs::path & p, const Args& ...args)
			{
				if (!p.empty( ))
					p.clear( );
				else
					p = construct(args...);
			});
		}
	}

	lazy_storage.write.reserve(empty_files_count);

	for (const auto& [CLASS_NAME, DATA] : files_to_generate)
	{
		const auto h = !DATA.h_cached.empty( );
		const auto cpp = !DATA.cpp_cached.empty( );

		if (!h && !cpp)
			continue;

		const auto make_file_writer = [&](const std::string_view& suffix)
		{
			return generated_classes_dir / _To_wstring(CLASS_NAME
#ifdef CHEAT_NETVARS_GENERATED_TAG
													   , generated_tag
#endif
													   , suffix);
		};

		lazy::file_writer writer_h, writer_cpp;
		if (h)
			writer_h = make_file_writer(suffix_h);
		if (cpp)
			writer_cpp = make_file_writer(suffix_cpp);

		for (const auto& [NETVAR_NAME, NETVAR_DATA] : DATA.netvars->items( ))
		{
			using namespace std::string_view_literals;
#ifdef CHEAT_NETVARS_LOG_STATIC_OFFSET
			const auto netvar_offset = netvar_info::offset.get(NETVAR_DATA);
#endif
			std::string_view netvar_type = NETVAR_DATA.find("type"sv)->get_ref<const std::string&>( );
			const auto netvar_type_pointer = netvar_type.ends_with('*');
			if (netvar_type_pointer)
				netvar_type.remove_suffix(1);
			const auto netvar_ret_char = netvar_type_pointer ? '*' : '&';

			const auto write_fn_head = [&](std::basic_ostream<char>& stream, bool inside_class, size_t indent = 0)
			{
				if (inside_class && indent > 0)
					std::fill_n(std::ostream_iterator<char>(stream), indent, '	');
				stream << netvar_type << netvar_ret_char << ' ';
				if (!inside_class)
					stream << CLASS_NAME << "::";
				stream << NETVAR_NAME << "( )";
				if (inside_class)
					stream << ';';
				stream << '\n';
			};
			const auto write_fn_body = [&](std::basic_ostream<char>& stream)
			{
				using namespace nstd;
				stream
					<< "{\n"
					<< '	'
#ifdef CHEAT_NETVARS_LOG_STATIC_OFFSET
					<< "constexpr auto offset = "
					<< netvar_offset
#else
					<< "static const auto offset = "
					//<< nstd::type_name<services_loader>( ) << "::get( ).deps( ).get<" << nstd::type_name<netvars>( ) << ">( ).at"
					<< "netvars::get_offset"
					<< "(\"" << CLASS_NAME << "\", \"" << NETVAR_NAME << "\")"
#endif
					<< ";\n"
					<< '	'
					<< "return " << type_name<basic_address>( ) << "(this) + offset;\n"
					<< "}\n\n";
			};

			if (h)
			{
				write_fn_head(writer_h, true);
			}
			if (cpp)
			{
				write_fn_head(writer_cpp, false);
				write_fn_body(writer_cpp);
			}
		}

		if (h)
			lazy_storage.write.push_back(std::move(writer_h));
		if (cpp)
			lazy_storage.write.push_back(std::move(writer_cpp));
	}

	console::log([&]
	{
		std::ostringstream msg;
		msg << "Netvars classes generation done.";
		if (!lazy_storage.remove.empty( ))
		{
			if (lazy_storage.remove.size( ) == 1 && lazy_storage.remove[0].all( ))
				msg << " Whole folder removed.";
			else
				msg << std::format(" Removed {} files.", lazy_storage.remove.size( ));
		}
		if (!lazy_storage.write.empty( ))
		{
			msg << std::format(" Created {} files.", lazy_storage.write.size( ));
		}
		return std::move(msg).str( );
	});

}
#endif

logs_data::~logs_data( )
{
	//moved
	if (dir.empty( ))
		return;

	const auto& path = reinterpret_cast<fs::path&>(dir);
	fs::create_directory(path);

	const auto& [file_name, file_ex] = file;
	std::wstring full_path;
	full_path.reserve(dir.size( ) + file_name.size( ) + file_ex.size( ));
	full_path += dir;
	full_path += file_name;
	full_path += file_ex;

	const auto file_new = data.view( );
	auto file_stored = std::ifstream(full_path, std::ios::binary | std::ios::ate);
	if (!file_stored.fail( ))
	{
		const auto size = static_cast<size_t>(file_stored.tellg( ));
		if (file_new.size( ) == size)
		{
			auto buff = std::make_unique<char[]>(size);
			if (file_stored.read(buff.get( ), size))
			{
				if (std::memcmp(buff.get( ), file_new.data( ), size) == 0)
					return;
			}
		}
		file_stored.close( );
	}

	std::ofstream(full_path) << file_new;
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

	data.data << std::setw(data.indent) << std::setfill(data.filler) << std::move(json);
}

void storage::generate_classes(classes_data& data)
{
	data.data.reserve(this->size( ));

	for (const netvar_table& table : *this)
	{
		if (table.empty( ))
			continue;

		classes_data::file_info h_info, cpp_info;
		auto& h = h_info.data;
		auto& cpp = cpp_info.data;

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

			const auto write_func_header = [=](std::basic_ostream<char>& stream, bool inside_class)
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

		data.data.push_back(std::move(h_info));
		data.data.push_back(std::move(cpp_info));
	}
}