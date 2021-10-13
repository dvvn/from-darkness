#include "data_dumper.h"

#include "config.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo interfaces.h"

#include "cheat/sdk/IVEngineClient.hpp"

#include <nstd/checksum.h>
#include <nstd/memory backup.h>

#include <fstream>
#include <set>

using namespace cheat::detail;

#define STRINGIZE_PATH(_PATH_) \
	_CONCAT(LR,_STRINGIZE(##(_PATH_)##))

dump_info cheat::detail::_Dump_netvars(const netvars_storage& netvars_data)
{
	const std::filesystem::path dumps_dir = STRINGIZE_PATH(CHEAT_NETVARS_DUMPS_DIR);

	const auto dirs_created = std::filesystem::create_directories(dumps_dir);

	constexpr auto get_file_name = []()-> std::filesystem::path
	{
		std::string version = csgo_interfaces::get_ptr( )->engine->GetProductVersionString( );
		std::ranges::replace(version, '.', '_');
		version.append(".json");
		return version;
	};

	const auto netvars_dump_file = dumps_dir / get_file_name( );
	const auto file_exists       = !dirs_created && exists(netvars_dump_file);

	if (!file_exists)
	{
		std::ofstream(netvars_dump_file) << std::setw(CHEAT_NETVARS_DUMP_FILE_INDENT) << std::setfill(CHEAT_NETVARS_DUMP_FILE_FILLER) << netvars_data;
		CHEAT_CONSOLE_LOG("Netvars dump done");
		return dump_info::created;
	}

	//------

	std::ostringstream netvars_data_as_text;
	netvars_data_as_text << std::setw(CHEAT_NETVARS_DUMP_FILE_INDENT) << std::setfill(CHEAT_NETVARS_DUMP_FILE_FILLER) << netvars_data;

	if (nstd::checksum(netvars_dump_file) != nstd::checksum(netvars_data_as_text))
	{
		std::ofstream(netvars_dump_file) << netvars_data_as_text.view( );
		CHEAT_CONSOLE_LOG("Netvars dump updated");
		return dump_info::updated;
	}

	CHEAT_CONSOLE_LOG("Netvars dump skipped");
	return dump_info::skipped;
}

void cheat::detail::_Generate_classes(dump_info info, netvars_storage& netvars_data, lazy_files_storage& lazy_storage)
{
	const std::filesystem::path generated_classes_dir = STRINGIZE_PATH(CHEAT_NETVARS_GENERATED_DIR);

	nstd::memory_backup<netvars_storage> netvars_data_backup;
	(void)netvars_data_backup;

	if (info == dump_info::skipped || info == dump_info::updated)
	{
		if (!exists(generated_classes_dir))
		{
			info = dump_info::created;
			goto _CREATE;
		}
		if (is_empty(generated_classes_dir))
		{
			info = dump_info::created;
			goto _WORK;
		}

		netvars_data_backup = netvars_data;
		size_t exists_count = 0;
		for (auto& entry : std::filesystem::directory_iterator(generated_classes_dir))
		{
			constexpr std::string_view suffix = _STRINGIZE(CHEAT_NETVARS_GENERATED_HEADER_POSTFIX);

			auto name   = entry.path( ).filename( ).string( );
			auto namesv = std::string_view(name);

			if (!namesv.ends_with(suffix))
				continue;

			namesv.remove_suffix(suffix.size( ));

			auto name_entry = netvars_data.find(std::string(namesv));
			if (name_entry == netvars_data.end( ))
			{
				info = dump_info::created;
				goto _REMOVE;
			}

			++exists_count;
			//netvars_data.erase(name_entry);

			const_cast<char&>(name_entry.key( )[0]) = '\0';
		}

		if (exists_count != netvars_data.size( ))
		{
			if (exists_count > 0)
			{
				netvars_storage to_create;
				for (auto& [key,val] : netvars_data.items( ))
				{
					if (key[0] == '\0')
						continue;

					to_create.emplace(const_cast<std::string&&>(key), std::move(val));
				}
				netvars_data = std::move(to_create);
			}
			info = dump_info::created;
			goto _WORK;
		}

		if (info == dump_info::skipped)
			return;
	}

_REMOVE:
	lazy_storage.remove.emplace_back(generated_classes_dir, true);
_CREATE:
	lazy_storage.create.emplace_back(generated_classes_dir);
_WORK:
	auto& lazy_writer = lazy_storage.write;

	lazy_writer.reserve(netvars_data.size( ) * 2);
	for (auto& [CLASS_NAME, NETVARS] : netvars_data.items( ))
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
#if !defined(CHEAT_NETVARS_RESOLVE_TYPE)
			runtime_assert("Unable to get dynamic includes!");
#else

			//auto includes = robin_hood::unordered_set<include_name, robin_hood::hash<std::string>>( );
			auto includes = std::set<include_name>( );

			const auto dir_path = std::filesystem::path(STRINGIZE_PATH(CHEAT_CSGO_SDK_DIR));
			for (auto& [netvar_name, netvar_data] : NETVARS.items( ))
			{
				const auto& netvar_type = netvar_data.at("type").get_ref<const std::string&>( );

				if (netvar_type.starts_with("std"))
				{
					const auto decayed = nstd::drop_namespaces(netvar_type);
					includes.emplace(decayed.substr(0, decayed.find('<')), true);
				}
				else if (netvar_type.starts_with("cheat"))
				{
					std::wstring full_path;
					std::wstring_view path_to_file;

#if 0
					full_path.reserve(netvar_type.size( ));
					for (auto itr = netvar_type.begin( ); itr != netvar_type.end( ); ++itr)
					{
						const auto c = *itr;
						if (c == ':')
						{
							++itr;
							full_path += std::filesystem::path::preferred_separator;
							path_to_file = full_path;
						}
						else
						{
							full_path += c;
						}
					}
#else
					runtime_assert(netvar_type.starts_with("cheat::csgo::"));
					path_to_file = STRINGIZE_PATH(cheat/sdk/);
					full_path.append(path_to_file);
					const auto name = nstd::drop_namespaces(netvar_type);
					full_path.append(name.begin( ), name.end( ));

#endif
					const auto test_file_name = std::wstring_view(full_path).substr(path_to_file.size( ));
					const auto checked_folder = STRINGIZE_PATH(_CONCAT(VS_SolutionDir, \impl\)) / std::filesystem::path(path_to_file);

					for (auto& entry : std::filesystem::directory_iterator(checked_folder))
					{
						if (!entry.is_regular_file( ))
							continue;

						auto full_file_name = std::wstring_view(entry.path( ).native( )).substr(checked_folder.native( ).size( ));
						if (!full_file_name.starts_with(test_file_name))
							continue;

						auto extension = full_file_name.substr(test_file_name.size( ));
						if (extension[0] != L'.')
							continue;

						if (extension[1] != L'h') //.cpp .cxx etc
							continue;

						std::string include;

						include.reserve(full_path.size( ) + extension.size( ));
						include.append(full_path.begin( ), full_path.end( ));
						include.append(extension.begin( ), extension.end( ));

						includes.emplace(std::move(include), false);
						break;
					}
				}
				else
				{
					runtime_assert(netvar_type.find("::") == netvar_type.npos, "Unknown namespace detected");

					if (netvar_type.ends_with("_t"))
						includes.emplace("cstdint", true);
				}
			}

			switch (includes.size( ))
			{
				case 0:
					return;
				case 1:
				{
					auto& first = *includes.begin( );
					source_add_include(first, first.global);
					break;
				}
				default:
				{
					const auto includes_adder = [&](bool global)
					{
						for (auto& in : includes)
						{
							if (in.global == global)
								source_add_include(in, global);
						}
					};

					includes_adder(false);
					includes_adder(true);

					break;
				}
			}

			source << __New_line;

#endif
		};

		source_add_include(CLASS_NAME + ".h");
		source_add_include("cheat/netvars/config.h");
		source << "#ifndef CHEAT_NETVARS_UPDATING" << __New_line;
		source_add_include("cheat/netvars/netvars.h");
		source_add_include("nstd/address.h", true);
		source << "#endif" << __New_line;
		source << __New_line;

		source_add_dynamic_includes( );

		//source << "using cheat::csgo::" << CLASS_NAME << ';' << __New_line;
		source << "using namespace cheat::csgo;" << __New_line;

		source << __New_line;

		for (auto& [NETVAR_NAME, NETVAR_DATA] : NETVARS.items( ))
		{
#ifdef CHEAT_NETVARS_DUMP_STATIC_OFFSET
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
			source << "#ifdef CHEAT_NETVARS_UPDATING" << __New_line;
			source << __Tab << std::format("return {}({}*)nullptr;", netvar_type_pointer ? "" : "*", netvar_type) << __New_line;
			source << "#else" << __New_line;
#ifdef CHEAT_NETVARS_DUMP_STATIC_OFFSET
			source << __Tab << format("auto addr = {}(this).add({});", _Address_class, netvar_offset) << __New_line;
#else
			source
					<< __Tab
					<< "static const auto offset = netvars::get_ptr( )->at"
					<< std::format("(\"{}\", \"{}\");", CLASS_NAME, NETVAR_NAME)
					<< __New_line;
			source << __Tab << "auto addr = " << _Address_class << "(this).add(offset);" << __New_line;
#endif
			source << __Tab << std::format("return addr.{}<{}>( );", netvar_type_pointer ? "ptr" : "ref", netvar_type) << __New_line;
			source << "#endif" << __New_line;
			source << '}' << __New_line;
		}

		lazy_writer.push_back(std::move(header));
		lazy_writer.push_back(std::move(source));
	}

	if (info == dump_info::created)
	{
		//write all without waiting
		lazy_storage = {};
	}

	(void)netvars_data_backup;
	CHEAT_CONSOLE_LOG("Netvars classes generation done");
}
