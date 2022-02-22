module;

#include "storage_includes.h"

#include "cheat/console/includes.h"

#include <nstd/mem/signature_includes.h>
#include <nstd/type name.h>
#include <nstd/unistring.h>
#include <nstd/checksum.h>
#include <nstd/unordered_set.h>
#include <nstd/enum_tools.h>
#include <nstd/unordered_map.h>

#include <fstream>
#include <regex>
#include <filesystem>

module cheat.netvars:data_dump;
import cheat.csgo.interfaces;
//import nstd.mem;
import nstd.container.wrapper;
import nstd.text.actions;

namespace cheat
{
	class netvars;
}

using namespace cheat;
using netvars_impl::netvars_storage;
namespace fs = std::filesystem;

#define CHEAT_NETVARS_LOG_FILE_INDENT 4
#define CHEAT_NETVARS_LOG_FILE_FILLER ' '
#define CHEAT_NETVARS_LOGS_DIR _CONCAT(VS_SolutionDir, \.dumps\netvars\)
#define CHEAT_NETVARS_GENERATED_DIR _CONCAT(VS_SolutionDir, \impl\cheat\csgo\interfaces_custom\)
#define CHEAT_CSGOSDK_DIR _CONCAT(VS_SolutionDir, \impl\cheat\csgo\)
#define CHEAT_NETVARS_GENERATED_HEADER_POSTFIX _h
#define CHEAT_NETVARS_GENERATED_SOURCE_POSTFIX _cpp
#define CHEAT_NETVARS_GENERATED_TAG _generated

#define STRINGIZE_PATH(_PATH_) _CONCAT(LR,_STRINGIZE(##(_PATH_)##))

//TEMPORARY
#define CHEAT_NETVARS_UPDATING 0

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

bool netvars_impl::log_netvars(console* logger, const char* game_version, const netvars_storage& root_netvars_data)
{
	const fs::path dumps_dir = STRINGIZE_PATH(CHEAT_NETVARS_LOGS_DIR);

	[[maybe_unused]]
	const auto dirs_created = create_directories(dumps_dir);

	const auto netvars_dump_file = dumps_dir / _To_wstring(_Correct_game_version_string(game_version), L".json");

#if !CHEAT_NETVARS_UPDATING
	const auto file_exists = !dirs_created && exists(netvars_dump_file);
	if (!file_exists)
#endif
	{
		auto file = std::ofstream(netvars_dump_file);
		file << std::setw(CHEAT_NETVARS_LOG_FILE_INDENT) << std::setfill(CHEAT_NETVARS_LOG_FILE_FILLER) << root_netvars_data;
		if (logger)
			logger->log("Netvars dump done");
		return true;
	}

	//------

	std::ostringstream netvars_data_as_text;
	netvars_data_as_text << std::setw(CHEAT_NETVARS_LOG_FILE_INDENT) << std::setfill(CHEAT_NETVARS_LOG_FILE_FILLER) << root_netvars_data;

	if (nstd::checksum(netvars_dump_file) != nstd::checksum(netvars_data_as_text))
	{
		std::ofstream(netvars_dump_file) << netvars_data_as_text.view( );
		if (logger)
			logger->log("Netvars dump updated");
		return true;
	}

	if (logger)
		logger->log("Netvars dump skipped");
	return false;
}

struct generated_file_data
{
	//generated_file_data(const std::wstring_view& name, const std::wstring_view& suffix) :name(name), suffix(suffix) { }

	//std::string class_name;
	//std::wstring file_name;
	fs::path h_cached;
	fs::path cpp_cached;
	const netvars_storage::value_type* netvars;
};

void netvars_impl::generate_classes(console* logger, bool recreate, netvars_storage& root_netvars_data, lazy::files_storage& lazy_storage)
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

	//nstd::mem::backup<netvars_storage> netvars_data_backup;

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
				if (logger)
					logger->log("Netvars classes are up-to-date.");
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
				stream
					<< "{\n"
					<< '	'
#ifdef CHEAT_NETVARS_LOG_STATIC_OFFSET
					<< "constexpr auto offset = "
					<< netvar_offset
#else
					<< "static const auto offset = "
					//<< nstd::type_name<services_loader>( ) << "::get( ).deps( ).get<" << nstd::type_name<netvars>( ) << ">( ).at"
					<< "cheat::get_netvar_offset"
					<< "(\"" << CLASS_NAME << "\", \"" << NETVAR_NAME << "\")"
#endif
					<< ";\n"
					<< '	'
					<< "return " << nstd::type_name<nstd::mem::basic_address>( ) << "(this) + offset;\n"
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

#if 0
	const auto make_file_writer = [&](const std::string_view& class_name, const std::string_view& suffix) -> lazy::file_writer
	{
		return generated_classes_dir / _Construct_append(class_name, suffix);
	};


	for (const auto& [CLASS_NAME_RAW, NETVARS] : root_netvars_data.items( ))
	{
		const auto CLASS_NAME = _Construct_append<std::string>(CLASS_NAME_RAW, generated_tag);

		auto header = make_file_writer(CLASS_NAME, suffix_h);
		auto source = make_file_writer(CLASS_NAME, suffix_cpp);

#ifdef CHEAT_NETVARS_GENERATE_MODULES
		//const auto module_head = std::format("module cheat.csgo.interfaces:{};\n", CLASS_NAME);
		const auto [global, add_tools, interfaces, add_math] = _Generate_includes_from_types(NETVARS);

		const auto write_global_includes = [&]<class ...T>(std::basic_ostream<char>&stream, const T& ...before)
		{
			constexpr auto write_before = sizeof...(T) > 0;
			const auto started = !global.empty( ) || write_before;
			if (started)
				stream << "module;\n\n";
			if constexpr (write_before)
				((stream << "#include " << before << "\n"), ...);
			for (auto& file : global)
				stream << "#include <" << file << ">\n";
			if (started)
				stream << '\n';
		};
		const auto write_head_module = [&](std::basic_ostream<char>& stream, bool visible)
		{
			if (visible)
				stream << "export ";
			stream << "module cheat.csgo.interfaces:" << CLASS_NAME << ";\n";
		};

		write_global_includes(source, "\"cheat/netvars/includes.h\"", "<nstd/mem/address_includes.h>");
		write_head_module(source, false);
		source << "import nstd.mem;\n";
		source << "import cheat.netvars;\n\n";
		source << "using namespace cheat;\n";
		source << "using namespace csgo;\n\n";

		write_global_includes(header);
		write_head_module(header, true);
		for (auto& ifc : interfaces)
			header << "export import :" << ifc << ";\n";
		if (add_tools)
			header << "export import cheat.csgo.tools;\n";
		if (add_math)
			header << "export import cheat.csgo.math;\n";
		if (!interfaces.empty( ) || add_tools || add_math)
			header << '\n';

#else
		//outdated 
		const auto source_add_include = [&](const std::string_view& file_name, bool global = false)
		{
			char l, r;

			if (global)
			{
				l = '<';
				r = '>';
			}
			else
			{
				l = '\"';
				r = '\"';
			}

			source
				<< "#include "
				<< l
				<< file_name
				<< r
				<< '\n';
		};

		source_add_include(CLASS_NAME + ".h");
		source_add_include("cheat/netvars/netvars.h");
		source_add_include("nstd/address.h", true);
		source << '\n';

		auto [local, global] = _Generate_includes_from_types(NETVARS);
		const auto write_includes = [&](std::vector<std::string>& data, bool is_global)
		{
			switch (data.size( ))
			{
			case 0:
				return false;
			case 1:
				source_add_include(data.front( ), is_global);
				break;
			default:
				std::ranges::sort(data);
				for (auto& d : data)
					source_add_include(d, is_global);
				break;
			}
			return true;
		};
		auto empty = true;
		if (write_includes(local, false))
			empty = false;
		if (write_includes(global, true))
			empty = false;
		if (!empty)
			source << '\n';

		//source << "using cheat::csgo::" << CLASS_NAME << ';' << '\n';
		source << "using namespace cheat::csgo;" << '\n';
		source << '\n';
#endif

		header << "export namespace cheat::csgo \n{\n";
		header << "	class " << CLASS_NAME << "\n	{\n";

		for (auto& [NETVAR_NAME, NETVAR_DATA] : NETVARS.items( ))
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

			const auto write_fn_head = [&](std::basic_ostream<char>& stream, bool inside_class)
			{
				if (inside_class)
					stream << "		";
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
				stream
					<< "{\n"
					<< '	'
#ifdef CHEAT_NETVARS_LOG_STATIC_OFFSET
					<< "constexpr auto offset = "
					<< netvar_offset
#else
					<< "static const auto offset = "
					<< nstd::type_name<services_loader>( ) << "::get( ).deps( ).get<" << nstd::type_name<netvars>( ) << ">( ).at"
					<< "(\"" << CLASS_NAME_RAW << "\", \"" << NETVAR_NAME << "\")"
#endif
					<< ";\n"
					<< '	'
					<< "auto addr = "
					<< nstd::type_name<nstd::mem::address>( )
					<< "(this).add(offset);\n"

					<< '	'
					//<< "return addr." << (netvar_type_pointer ? "ptr" : "ref") << '<' << netvar_type << ">( );\n"
					<< "return addr." << (netvar_type_pointer ? "ptr" : "ref") << "( );\n"
					<< "}\n\n";
			};

			write_fn_head(header, true);
			write_fn_head(source, false);
			write_fn_body(source);

#if 0
			//outdated
			header << std::format("{}{} {}( );", netvar_type, netvar_ret_char, NETVAR_NAME) << '\n';
			source << std::format("{}{} {}::{}( )", netvar_type, netvar_ret_char, CLASS_NAME, NETVAR_NAME) << '\n';
			source << '{' << '\n';
#ifdef CHEAT_NETVARS_LOG_STATIC_OFFSET
			source << '	' << format("auto addr = {}(this).add({});", _Address_class, netvar_offset) << '\n';
#else
			source << '	'
				<< "static const auto offset = netvars_impl::get( )->at"
				<< std::format("(\"{}\", \"{}\");", CLASS_NAME, NETVAR_NAME)
				<< '\n';
			source << '	' << "auto addr = " << _Address_class << "(this).add(offset);" << '\n';
#endif
			source << '	' << std::format("return addr.{}<{}>( );", netvar_type_pointer ? "ptr" : "ref", netvar_type) << '\n';
			source << '}' << '\n';
#endif
		}

		header << "\n	};\n}\n";

		lazy_storage.write.push_back(std::move(header));
		lazy_storage.write.push_back(std::move(source));
	}
#endif

	if (logger)
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

		logger->log(std::move(msg));
}
}
