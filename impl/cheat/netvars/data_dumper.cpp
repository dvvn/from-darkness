// ReSharper disable once CppUnusedIncludeDirective
#include "data_filler.h"

#ifdef CHEAT_NETVARS_RESOLVE_TYPE

#include "data_dumper.h"
#include "lazy.h"
#include "storage.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo_interfaces.h"

#include "cheat/csgo/IVEngineClient.hpp"

#include <nstd/unistring.h>
#include <nstd/checksum.h>
#include <nstd/mem/backup.h>
#include <nstd/custom_types.h>
#include NSTD_UNORDERED_SET_INCLUDE

#include <fstream>
#include <ranges>
#include <regex>

using namespace cheat::detail;
using netvars::log_info;

using namespace std::string_view_literals;
using namespace std::string_literals;

namespace fs = std::filesystem;

#define STRINGIZE_PATH(_PATH_) \
	_CONCAT(LR,_STRINGIZE(##(_PATH_)##))

#define CHEAT_NETVARS_LOG_FILE_INDENT 4
#define CHEAT_NETVARS_LOG_FILE_FILLER ' '
#define CHEAT_NETVARS_LOGS_DIR _CONCAT(VS_SolutionDir, \.out\netvars\)
#define CHEAT_NETVARS_GENERATED_DIR _CONCAT(VS_SolutionDir, \impl\cheat\csgo\generated\)
#define CHEAT_CSGOSDK_DIR _CONCAT(VS_SolutionDir, \impl\cheat\csgo\)
#define CHEAT_NETVARS_GENERATED_HEADER_POSTFIX _h
#define CHEAT_NETVARS_GENERATED_SOURCE_POSTFIX _cpp

log_info netvars::log_netvars(const netvars_root_storage& netvars_data)
{
	const fs::path dumps_dir = STRINGIZE_PATH(CHEAT_NETVARS_LOGS_DIR);

	[[maybe_unused]] const auto dirs_created = create_directories(dumps_dir);

	const auto netvars_dump_file = [&]
	{
		const std::string_view version = csgo_interfaces::get( )->engine->GetProductVersionString( );
		std::wstring file_name;
		file_name.reserve(version.size( ));
		for (const auto c: version)
			file_name += static_cast<wchar_t>(c == '.' ? '_' : c);
		return dumps_dir / fs::path(std::move(file_name));
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
}

using include_name = std::pair<std::string, bool>;

static auto _Get_includes(std::string_view type, bool detect_duplicates)
{
	std::vector<include_name> result;
	NSTD_UNORDERED_SET<std::string_view> results_used; //skip stuff like array<array<X,....

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

		if (str.find(':') != str.npos)
		{
			constexpr auto std_namespace   = "std::"sv;
			constexpr auto cheat_namespace = "cheat::"sv;

			auto global = !str.starts_with(cheat_namespace);

			if (global)
			{
				if (str.starts_with(std_namespace))
					str.remove_prefix(std_namespace.size( ));
			}

			std::string include;
			for (const auto c: str
							   | std::views::split("::"sv)
							   | std::views::transform(subrange_to_string_view))
			{
				if (!include.empty( ))
					include += '/';
				include += c;
			}

			result.emplace_back(std::move(include), global);
		}
		else if (str.ends_with("_t"))
		{
			result.emplace_back("cstdint"s, true);
		}
	}

	return result;
}

template <bool Unsafe = true, typename T>
static T _Parse_filename_fast(const T& str)
{
	auto end = str._Unchecked_end( );
	auto itr = end;
	for (;;)
	{
		if constexpr (!Unsafe)
		{
			if (itr == str._Unchecked_begin( ))
				return str;
		}

		const wchar_t chr = *--itr;
		if (std::_Is_slash(chr))
		{
			++itr;
			break;
		}
	}

	return T(itr, end);
}

template <typename T>
static T _Parse_extension_fast(const T& str)
{
	auto dot = str.rfind(static_cast<typename T::value_type>('.'));
	if (dot == str.npos)
		return str;
	return str.substr(dot);
}

static auto _Get_possible_headers(const fs::path& path)
{
	std::vector<std::string> out;

	for (const auto& entry: fs::directory_iterator(path))
	{
		if (!entry.is_regular_file( ))
			continue;

		const auto& str = entry.path( ).native( );

		const auto filename  = _Parse_filename_fast<false>(str);
		const auto extension = _Parse_extension_fast(filename);

		if (!extension.starts_with(L".h"sv))
			continue;

		out.push_back({filename.begin( ), filename.end( )});
	}

	return out;
}

struct generated_includes
{
	std::vector<std::string> local, global;
};

static auto _Generate_includes_from_types(const netvars::netvars_root_storage& storage)
{
	generated_includes includes;
	const fs::path csgosdk_dir = STRINGIZE_PATH(CHEAT_CSGOSDK_DIR);

	NSTD_UNORDERED_SET<std::string> used_includes_cache;
	const auto headers_cache = _Get_possible_headers(csgosdk_dir);

	for (auto& [netvar_name, netvar_data]: storage.items( ))
	{
		const std::string_view netvar_type = netvar_data.find("type"sv)->get_ref<const std::string&>( );

		//netvar type already processed
		if (!used_includes_cache.emplace(netvar_type).second)
			continue;
		if (netvar_type.find(':') == netvar_type.npos && !netvar_type.ends_with("_t"sv))
			continue;

		const auto templates_count = std::ranges::count(netvar_type, '<');
		auto types_found           = _Get_includes(netvar_type, templates_count > 1);
		for (auto& [incl, global]: types_found)
		{
			if (templates_count > 0 && !used_includes_cache.emplace(incl).second)
				continue;

			if (global)
			{
				includes.global.push_back(std::move(incl));
			}
			else
			{
				const auto incl_sv = _Parse_filename_fast<false, std::string_view>(incl);
				for (const std::string_view cached: headers_cache)
				{
					if (!cached.starts_with(incl_sv))
						continue;
					const auto extension = cached.substr(incl_sv.size( ));
					if (!extension.starts_with('.'))
						continue;
					incl.append(extension.begin( ), extension.end( ));
					includes.local.push_back(std::move(incl));
				}
			}
		}
	}

	return includes;
}

struct generated_file
{
	using string_type = std::wstring;

	template <typename T>
	generated_file(T&& arg)
		: str(/*std::wstring*/std::forward<T>(arg))
	{
	}

	bool cpp = false;
	bool h   = false;
	string_type str;
};

bool operator==(const generated_file& l, const generated_file& r)
{
	//return r.str.size( ) == l.str.size( ) && _Parse_filename_fast(r.str) == _Parse_filename_fast(l.str);
	return r.str == l.str;
}

template < >
struct NSTD_UNORDERED_HASH<generated_file>
{
	size_t operator()(const generated_file& f) const
	{
		return std::invoke(NSTD_UNORDERED_HASH<std::wstring_view>( ), /*_Parse_filename_fast*/(f.str));
	}
};

static auto _Get_generated_files(const fs::path& folder, const std::wstring_view& cpp, const std::wstring_view& h)
{
	NSTD_UNORDERED_SET<generated_file> storage;

	for (const auto& entry: fs::directory_iterator(folder))
	{
		std::wstring_view str = entry.path( ).native( );

		const auto fix_add_str = [&](const std::wstring_view& postfix)
		{
			auto& fstr = folder.native( );
			str.remove_prefix(fstr.size( ));
			/*if (!std::_Is_slash(fstr.back( )))
				str.remove_prefix(1);*/
			str.remove_suffix(postfix.size( ));
			return storage.emplace(str);
		};

		if (str.ends_with(cpp))
			fix_add_str(cpp).first->cpp = true;
		else if (str.ends_with(h))
			fix_add_str(h).first->h = true;
	}

	return storage;
}

void netvars::generate_classes(log_info info, netvars_root_storage& netvars_data, lazy_files_storage& lazy_storage)
{
	const fs::path generated_classes_dir = STRINGIZE_PATH(CHEAT_NETVARS_GENERATED_DIR);
	runtime_assert(std::_Is_slash(generated_classes_dir.native().back()));

	constexpr std::string_view suffix_h   = _STRINGIZE(CHEAT_NETVARS_GENERATED_HEADER_POSTFIX);
	constexpr std::string_view suffix_cpp = _STRINGIZE(CHEAT_NETVARS_GENERATED_SOURCE_POSTFIX);

	constexpr std::wstring_view wsuffix_h   = _CRT_WIDE(_STRINGIZE(CHEAT_NETVARS_GENERATED_HEADER_POSTFIX));
	constexpr std::wstring_view wsuffix_cpp = _CRT_WIDE(_STRINGIZE(CHEAT_NETVARS_GENERATED_SOURCE_POSTFIX));

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

		const auto generated_files = _Get_generated_files(generated_classes_dir, wsuffix_cpp, wsuffix_h);
		for (auto& [cpp,h,file]: generated_files)
		{
			const auto add_to_erase0 = [&](const std::wstring_view& postfix)
			{
				std::wstring path;
				path.reserve(generated_classes_dir.native( ).size( ) + file.size( ) + postfix.size( ));
				path.append(generated_classes_dir.native( ));
				path.append(file);
				path.append(postfix);

				lazy_storage.remove.emplace_back(std::move(path), false);
			};
			const auto add_to_erase = [&]
			{
				if (!cpp)
					add_to_erase0(wsuffix_cpp);
				if (!h)
					add_to_erase0(wsuffix_h);
			};

			if (!cpp || !h)
			{
				add_to_erase( );
				continue;
			}

			const auto filechr    = nstd::unistring<char>(file);
			const auto filechr_sv = std::string_view(reinterpret_cast<const char*>(filechr._Unchecked_begin( )), filechr.size( ));
			auto name_entry       = netvars_data.find(filechr_sv);
			if (name_entry == netvars_data.end( ))
			{
				add_to_erase( );
				continue;
			}

			netvars_data.erase(name_entry);
		}

		if (!netvars_data.empty( ) && (netvars_data_backup.get( ).size( ) != netvars_data.size( ) || !lazy_storage.remove.empty( )))
		{
			info = log_info::created;
			goto _WORK;
		}

		return;
	}

_REMOVE:
	lazy_storage.remove.emplace_back(generated_classes_dir, true);
_CREATE:
	lazy_storage.create.emplace_back(generated_classes_dir);
_WORK:
	lazy_storage.write.reserve(netvars_data.size( ) * 2);

	constexpr auto netvars_data_items = []<class T>(const T& data)
	{
		return data.items( );
	};

	const auto make_file_writer = [&](const std::string_view& class_name, const std::string_view& suffix)
	{
		std::wstring tmp;
		tmp.reserve(class_name.size( ) + suffix.size( ));
		tmp.append(class_name.begin( ), class_name.end( ));
		tmp.append(suffix.begin( ), suffix.end( ));

		const auto tmp_fs = fs::path(std::move(tmp));
		return lazy_file_writer(generated_classes_dir / tmp_fs);
	};

	for (auto& [CLASS_NAME, NETVARS]: netvars_data_items(netvars_data))
	{
		// ReSharper disable CppInconsistentNaming
		// ReSharper disable CppTooWideScope
		constexpr auto __New_line = '\n';
		constexpr auto __Tab      = '	';
		// ReSharper restore CppTooWideScope
		// ReSharper restore CppInconsistentNaming

		auto header = make_file_writer(CLASS_NAME, suffix_h);
		auto source = make_file_writer(CLASS_NAME, suffix_cpp);

		const auto source_add_include = [&](const std::string_view& file_name, bool global = false)
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
			auto [local, global]      = _Generate_includes_from_types(static_cast<const netvars_root_storage&>(NETVARS));
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
						for (auto& d: data)
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

		for (auto& [NETVAR_NAME, NETVAR_DATA]: netvars_data_items(NETVARS))
		{
#ifdef CHEAT_NETVARS_LOG_STATIC_OFFSET
			const auto netvar_offset = netvar_info::offset.get(NETVAR_DATA);
#endif
			std::string_view netvar_type   = NETVAR_DATA.find("type"sv)->get_ref<const std::string&>( );
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

		lazy_storage.write.push_back(std::move(header));
		lazy_storage.write.push_back(std::move(source));
	}

#ifdef CHEAT_HAVE_CONSOLE
	std::string removed_msg;
	if (!lazy_storage.remove.empty( ))
	{
		if (lazy_storage.remove.size( ) == 1 && lazy_storage.remove[0].all( ))
			removed_msg = " Whole folder removed."s;
		else
			removed_msg = std::format(" Removed {} files."sv, lazy_storage.remove.size( ));
	}
	std::string created_msg;
	if (!lazy_storage.write.empty( ))
	{
		created_msg = std::format(" Created {} files."sv, lazy_storage.write.size( ));
	}

#endif

	CHEAT_CONSOLE_LOG(std::format("Netvars classes generation done.{}{}",removed_msg, created_msg));

	if (info == log_info::created)
	{
		//write all without waiting
		auto dummy = lazy_files_storage( );
		std::swap(lazy_storage, dummy);
	}
}

#endif
