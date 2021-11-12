// ReSharper disable once CppUnusedIncludeDirective
#include "data_filler.h"
#ifdef CHEAT_NETVARS_RESOLVE_TYPE

#include "data_dumper.h"
#include "lazy.h"
#include "storage.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo_interfaces.h"

#include "cheat/csgo/IVEngineClient.hpp"

#include <nstd/checksum.h>
#include <nstd/mem/backup.h>
#include <nstd/custom_types.h>
#include NSTD_UNORDERED_SET_INCLUDE

#include <fstream>
#include <ranges>
#include <regex>

using namespace cheat::detail;
using netvars::log_info;
using netvars::include_name;

#define STRINGIZE_PATH(_PATH_) \
	_CONCAT(LR,_STRINGIZE(##(_PATH_)##))

#define CHEAT_NETVARS_LOG_FILE_INDENT 4
#define CHEAT_NETVARS_LOG_FILE_FILLER ' '
#define CHEAT_NETVARS_LOGS_DIR _CONCAT(VS_SolutionDir, \.out\netvars\)
#define CHEAT_NETVARS_GENERATED_DIR _CONCAT(VS_SolutionDir, \impl\cheat\csgo\generated\)
#define CHEAT_NETVARS_GENERATED_HEADER_POSTFIX _h
#define CHEAT_NETVARS_GENERATED_SOURCE_POSTFIX _cpp

log_info netvars::log_netvars(const netvars_root_storage& netvars_data)
{
	const std::filesystem::path dumps_dir = STRINGIZE_PATH(CHEAT_NETVARS_LOGS_DIR);

	[[maybe_unused]] const auto dirs_created = create_directories(dumps_dir);

	const auto netvars_dump_file = [&]
	{
		const std::string_view version = csgo_interfaces::get( )->engine->GetProductVersionString( );
		std::filesystem::path file_name;
		if (version.size( ) > file_name.native( ).capacity( ))
		{
			auto& str = const_cast<std::wstring&>(file_name.native( ));
			str.reserve(version.size( ));
		}
		for (const auto c: version)
			file_name += static_cast<wchar_t>(c == '.' ? '_' : c);
		return dumps_dir / file_name;
	}( );
#if !CHEAT_NETVARS_UPDATING
	const auto file_exists = !dirs_created && exists(netvars_dump_file);
	if (!file_exists)
#endif
	{
		std::ofstream(netvars_dump_file) << std::setw(CHEAT_NETVARS_LOG_FILE_INDENT) << std::setfill(CHEAT_NETVARS_LOG_FILE_FILLER) << netvars_data;
		CHEAT_CONSOLE_LOG("Netvars dump done");
		return log_info::created;
	}

	//------

	// ReSharper disable CppUnreachableCode
	std::ostringstream netvars_data_as_text;
	netvars_data_as_text << std::setw(CHEAT_NETVARS_LOG_FILE_INDENT) << std::setfill(CHEAT_NETVARS_LOG_FILE_FILLER) << netvars_data;

	if (nstd::checksum(netvars_dump_file) != nstd::checksum(netvars_data_as_text))
	{
		std::ofstream(netvars_dump_file) << netvars_data_as_text.view( );
		CHEAT_CONSOLE_LOG("Netvars dump updated");
		return log_info::updated;
	}

	CHEAT_CONSOLE_LOG("Netvars dump skipped");
	return log_info::skipped;
	// ReSharper restore CppUnreachableCode
}

static auto _Get_includes(std::string_view type, bool detect_duplicates)
{
	std::vector<include_name> result;
	NSTD_UNORDERED_SET<std::string_view> results_used; //skip stuff like array<array<X,....

	using namespace std::string_view_literals;
	using namespace std::string_literals;

	const auto pat   = std::regex("[\\w:*]+");
	const auto start = std::regex_iterator(type.begin( ), type.end( ), pat);
	const auto end   = decltype(start)( );

	constexpr auto subrange_to_string_view = []<class T>(T&& srng)-> std::string_view
	{
		return {std::addressof(*srng.begin( )), static_cast<size_t>(std::ranges::distance(srng))};
	};

	for (const auto& match: std::ranges::subrange(start, end))
	{
#if 0
		_NODISCARD string_type str(size_type _Sub = 0) const {
			return string_type((*this)[_Sub]);
		}
#endif
		auto& match0 = match[0];
#if 0
		_NODISCARD string_type str() const { // convert matched text to string
			const _Mybase _Range(_Effective_range());
			return string_type(_Range.first, _Range.second);
		}
#endif
		auto str = std::string_view(match0.first, match0.second); //match.str()

		//-----

		if (detect_duplicates && !results_used.emplace(str).second)
			continue;

		include_name info;

		if (str.find(':') != str.npos)
		{
			constexpr auto std_namespace   = "std::"sv;
			constexpr auto cheat_namespace = "cheat::"sv;

			info.global = !str.starts_with(cheat_namespace);

			if (info.global)
			{
				if (str.starts_with(std_namespace))
					str.remove_prefix(std_namespace.size( ));
			}

			for (const auto c: str
							   | std::views::split("::"sv)
							   | std::views::transform(subrange_to_string_view))
			{
				if (!info.empty( ))
					info += '/';
				info += c;
			}
		}
		else if (str.ends_with("_t"))
		{
			info = {"cstdint"s, true};
		}
		else
		{
			continue;
		}

		result.push_back(std::move(info));
	}

	return result;
}

void netvars::generate_classes(log_info info, netvars_root_storage& netvars_data, lazy_files_storage& lazy_storage)
{
	const std::filesystem::path generated_classes_dir = STRINGIZE_PATH(CHEAT_NETVARS_GENERATED_DIR);

	nstd::mem::backup<netvars_root_storage> netvars_data_backup;
	(void)netvars_data_backup;

	if (info == log_info::skipped || info == log_info::updated)
	{
		if (!exists(generated_classes_dir))
		{
			info = log_info::created;
			goto _CREATE;
		}
		if (is_empty(generated_classes_dir))
		{
			info = log_info::created;
			goto _WORK;
		}

		netvars_data_backup = netvars_data;
		size_t exists_count = 0;
		for (const auto& entry: std::filesystem::directory_iterator(generated_classes_dir))
		{
			constexpr std::string_view suffix = _STRINGIZE(CHEAT_NETVARS_GENERATED_HEADER_POSTFIX);

			const auto name = entry.path( ).filename( ).string( );
			auto namesv     = std::string_view(name);

			if (!namesv.ends_with(suffix))
				continue;
			namesv.remove_suffix(suffix.size( ));

			auto name_entry = netvars_data.find(namesv);
			if (name_entry == netvars_data.end( ))
			{
				info = log_info::created;
				goto _REMOVE;
			}

			++exists_count;
			const_cast<char&>(name_entry.key( )[0]) = '\0';
		}

		if (exists_count != netvars_data.size( ))
		{
			if (exists_count > 0)
			{
				netvars_storage to_create;
				for (auto& [key, val]: netvars_data.items( ))
				{
					if (key[0] == '\0')
						continue;

					to_create.emplace(const_cast<std::string&&>(key), std::move(val));
				}
				netvars_data = std::move(to_create);
			}
			info = log_info::created;
			goto _WORK;
		}

		if (info == log_info::skipped)
			return;
	}

_REMOVE:
	lazy_storage.remove.emplace_back(generated_classes_dir, true);
_CREATE:
	lazy_storage.create.emplace_back(generated_classes_dir);
_WORK:
	auto& lazy_writer = lazy_storage.write;

	lazy_writer.reserve(netvars_data.size( ) * 2);
	for (auto& [CLASS_NAME, NETVARS]: netvars_data.items( ))
	{
		// ReSharper disable CppInconsistentNaming
		// ReSharper disable CppTooWideScope
		constexpr auto __New_line = '\n';
		constexpr auto __Tab      = '	';
		// ReSharper restore CppTooWideScope
		// ReSharper restore CppInconsistentNaming

		auto header = lazy_file_writer(generated_classes_dir / (CLASS_NAME + _STRINGIZE(CHEAT_NETVARS_GENERATED_HEADER_POSTFIX)));
		auto source = lazy_file_writer(generated_classes_dir / (CLASS_NAME + _STRINGIZE(CHEAT_NETVARS_GENERATED_SOURCE_POSTFIX)));

		const auto source_add_include = [&source](const std::string_view& file_name, bool global = false)
		{
			source <<
					"#include "
					<< (global ? '<' : '"')
					<< file_name
					<< (global ? '>' : '"')
					<< __New_line;
		};

		const auto source_add_dynamic_includes = [&]
		{
			std::vector<std::string> includes_local, includes_global;
			auto includes_cache = NSTD_UNORDERED_SET<std::string>( );

			const auto cheat_impl_dir = std::filesystem::path(STRINGIZE_PATH(_CONCAT(VS_SolutionDir, \impl\)));
			for (auto& [netvar_name, netvar_data]: NETVARS.items( ))
			{
				const std::string_view netvar_type = netvar_data.at("type").get_ref<const std::string&>( );

				//netvar type already processed
				if (!includes_cache.emplace(netvar_type).second)
					continue;
				if (netvar_type.find(':') == netvar_type.npos && !netvar_type.ends_with("_t"))
					continue;

				const auto netvar_type_templates_count = std::ranges::count(netvar_type, '<');
				auto types_found                       = _Get_includes(netvar_type, netvar_type_templates_count > 1);
				for (auto& incl: types_found)
				{
					if (netvar_type_templates_count > 0)
					{
						if (!includes_cache.emplace(incl).second)
							continue;
					}

					if (incl.global)
					{
						includes_global.push_back(std::move(incl));
					}
					else
					{
						const auto wincl                      = std::wstring(incl.begin( ), incl.end( ));
						const auto estimated_filename         = std::wstring_view(wincl).substr(wincl.rfind('/') + 1); //add 1 to skip '/'
						const auto estimated_dir_fs           = (cheat_impl_dir / wincl).parent_path( );
						const auto& estimated_dir             = estimated_dir_fs.native( );
						const size_t estimated_dir_extra_size = estimated_dir.ends_with('\\') || estimated_dir.ends_with('/') ? 0 : 1;

						for (const auto& entry: std::filesystem::directory_iterator(estimated_dir_fs))
						{
							if (!entry.is_regular_file( ))
								continue;

							const auto full_file_name = std::wstring_view(entry.path( ).native( )).substr(estimated_dir.size( ) + estimated_dir_extra_size);

							if (full_file_name.size( ) < estimated_filename.size( ) + 2) // 2->".h"
								continue;

							const auto extension = full_file_name.substr(estimated_filename.size( ));
							if (extension[0] != L'.')
								continue;
							if (extension[1] != L'h') //only .h*** allowed
								continue;

							if (!full_file_name.starts_with(estimated_filename))
								continue;

							incl.append(extension.begin( ), extension.end( ));
							includes_local.push_back(std::move(incl));
							break;
						}
					}
				}
			}

			const auto write_includes = [&](std::vector<std::string>& data, bool global)
			{
				switch (data.size( ))
				{
					case 0:
						return false;
					case 1:
						source_add_include(data.front( ), global);
						break;
					default:
						std::ranges::sort(data);
						for (auto& d: data)
							source_add_include(d, global);
						break;
				}
				return true;
			};

			auto empty = true;

			if (write_includes(includes_local, false))
				empty = false;
			if (write_includes(includes_global, true))
				empty = false;

			if (!empty)
				source << __New_line;
		};

		source_add_include(CLASS_NAME + ".h");
		source_add_include("cheat/netvars/netvars.h");
		source_add_include("nstd/address.h", true);
		source << __New_line;

		source_add_dynamic_includes( );

		//source << "using cheat::csgo::" << CLASS_NAME << ';' << __New_line;
		source << "using namespace cheat::csgo;" << __New_line;

		source << __New_line;

		for (auto& [NETVAR_NAME, NETVAR_DATA]: NETVARS.items( ))
		{
#ifdef CHEAT_NETVARS_LOG_STATIC_OFFSET
			const auto netvar_offset = netvar_info::offset.get(NETVAR_DATA);
#endif
			std::string_view netvar_type   = NETVAR_DATA.at("type").get_ref<const std::string&>( );
			const auto netvar_type_pointer = netvar_type.ends_with('*');
			if (netvar_type_pointer)
				netvar_type.remove_suffix(1);

			const auto netvar_ret_char = netvar_type_pointer ? '*' : '&';

			// ReSharper disable once CppInconsistentNaming
			constexpr auto _Address_class = nstd::type_name<nstd::address>;

			header << std::format("{}{} {}( );", netvar_type, netvar_ret_char, NETVAR_NAME) << __New_line;
			source << std::format("{}{} {}::{}( )", netvar_type, netvar_ret_char, CLASS_NAME, NETVAR_NAME) << __New_line;
			source << '{' << __New_line;
#ifdef CHEAT_NETVARS_LOG_STATIC_OFFSET
			source << __Tab << format("auto addr = {}(this).add({});", _Address_class, netvar_offset) << __New_line;
#else
			source << __Tab
					<< "static const auto offset = netvars::get( )->at"
					<< std::format("(\"{}\", \"{}\");", CLASS_NAME, NETVAR_NAME)
					<< __New_line;
			source << __Tab << "auto addr = " << _Address_class << "(this).add(offset);" << __New_line;
#endif
			source << __Tab << std::format("return addr.{}<{}>( );", netvar_type_pointer ? "ptr" : "ref", netvar_type) << __New_line;
			source << '}' << __New_line;
		}

		lazy_writer.push_back(std::move(header));
		lazy_writer.push_back(std::move(source));
	}

	if (info == log_info::created)
	{
		//write all without waiting
		auto dummy = lazy_files_storage( );
		std::swap(lazy_storage, dummy);
	}

	CHEAT_CONSOLE_LOG("Netvars classes generation done");
}

#endif
