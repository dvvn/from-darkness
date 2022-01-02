module;

#include "storage_includes.h"

#include "cheat/console/includes.h"

#include <nstd/signature_includes.h>
#include <nstd/type name.h>
#include <nstd/unistring.h>
#include <nstd/checksum.h>
#include <nstd/unordered_set.h>
#include <nstd/ranges.h>
#include <nstd/format.h>
#include <nstd/enum_tools.h>

#include <fstream>
#include <regex>

module cheat.netvars:data_dump;
import cheat.console;
import cheat.csgo.interfaces;
import nstd.mem.backup;
import nstd.signature;

import cheat.csgo.structs.EngineClient;

using namespace cheat;
using netvars_impl::log_info;

namespace fs = std::filesystem;

#define CHEAT_NETVARS_LOG_FILE_INDENT 4
#define CHEAT_NETVARS_LOG_FILE_FILLER ' '
#define CHEAT_NETVARS_LOGS_DIR _CONCAT(VS_SolutionDir, \.out\netvars\)
#define CHEAT_NETVARS_GENERATED_DIR _CONCAT(VS_SolutionDir, \impl\cheat\csgo\generated\)
#define CHEAT_CSGOSDK_DIR _CONCAT(VS_SolutionDir, \impl\cheat\csgo\)
#define CHEAT_NETVARS_GENERATED_HEADER_POSTFIX _h
#define CHEAT_NETVARS_GENERATED_SOURCE_POSTFIX _cpp

//#define CHEAT_NETVARS_GENERATE_MODULES

#define STRINGIZE_PATH(_PATH_) _CONCAT(LR,_STRINGIZE(##(_PATH_)##))

//TEMPORARY
#define CHEAT_NETVARS_UPDATING 0

log_info netvars_impl::log_netvars(const netvars_storage& root_netvars_data)
{
	const fs::path dumps_dir = STRINGIZE_PATH(CHEAT_NETVARS_LOGS_DIR);

	[[maybe_unused]] const auto dirs_created = create_directories(dumps_dir);

	const auto netvars_dump_file = [&]
	{
		const std::string_view version = csgo_interfaces::get( )->engine->GetProductVersionString( );
		constexpr std::wstring_view extension = L".json";

		std::wstring file_name;
		file_name.reserve(version.size( ) + extension.size( ));
		for (const auto c : version)
			file_name += static_cast<wchar_t>(c == '.' ? '_' : c);
		file_name += extension;

		return dumps_dir / std::move(file_name);
	}();
#if !CHEAT_NETVARS_UPDATING
	const auto file_exists = !dirs_created && exists(netvars_dump_file);
	if (!file_exists)
#endif
	{
		auto file = std::ofstream(netvars_dump_file);
		file << std::setw(CHEAT_NETVARS_LOG_FILE_INDENT) << std::setfill(CHEAT_NETVARS_LOG_FILE_FILLER) << root_netvars_data;
		console::get( ).log("Netvars dump done");
		return log_info::created;
	}

	//------

	std::ostringstream netvars_data_as_text;
	netvars_data_as_text << std::setw(CHEAT_NETVARS_LOG_FILE_INDENT) << std::setfill(CHEAT_NETVARS_LOG_FILE_FILLER) << root_netvars_data;

	if (nstd::checksum(netvars_dump_file) != nstd::checksum(netvars_data_as_text))
	{
		std::ofstream(netvars_dump_file) << netvars_data_as_text.view( );
		console::get( ).log("Netvars dump updated");
		return log_info::updated;
	}

	console::get( ).log("Netvars dump skipped");
	return log_info::skipped;
}

template <bool Unsafe = true, typename T>
static T _Parse_filename(const T& str)
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

		const auto chr = *--itr;

		if (std::_Is_slash(static_cast<wchar_t>(chr)))
		{
			++itr;
			break;
		}
	}

	return T(itr, end);
}

template <typename T>
static T _Parse_extension(const T& str)
{
	auto dot = str.rfind(static_cast<typename T::value_type>('.'));
	if (dot == str.npos)
		return str;
	return str.substr(dot);
}

template <typename T, typename ...Args>
static void _Reserve_append(T& obj, const Args&...args)
{
	runtime_assert(obj.empty( ));
	obj.reserve((args.size( ) + ...));
	(obj.append(args.begin( ), args.end( )), ...);
}

template <class T>
concept has_traits_type = requires { typename T::traits_type; };

template <typename T>
struct extract_traits
{
	using type = std::char_traits<typename T::value_type>;
};

template <has_traits_type T>
struct extract_traits<T>
{
	using type = typename T::traits_type;
};

struct generated_file_type
{
#ifndef CHEAT_NETVARS_GENERATE_MODULES
	enum types :uint8_t
	{
		UNSET
		, CPP = 1 << 0
		, H = 1 << 1
	};

	types type;

	generated_file_type(size_t type = UNSET)
		: type(static_cast<types>(type))
	{
	}

#endif
};

template <std::_Has_member_value_type Str>
struct generated_file : generated_file_type
{
	using value_type = typename Str::value_type;
	using traits_type = typename extract_traits<Str>::type;

	using string_type = Str;
	using string_view_type = std::basic_string_view<value_type, traits_type>;

	generated_file(string_view_type full_str, const string_view_type& prefix, const string_view_type& postfix)
	{
		full_str.remove_prefix(prefix.size( ));
		full_str.remove_suffix(postfix.size( ));
		str.assign(full_str.begin( ), full_str.end( ));
	}

	string_type str;

	bool operator==(const generated_file& other) const
	{
		return str == other.str;
	}
};

#ifdef CHEAT_NETVARS_GENERATE_MODULES
static auto _Get_generated_files(const fs::path& folder, const std::wstring_view& postfix)
{
	std::vector<generated_file> storage;

	for (const auto& entry : fs::directory_iterator(folder))
	{
		const auto& str = entry.path( ).native( );
		if (str.ends_with(postfix))
			storage.emplace_back(str, folder.native( ), postfix);
	}

	return storage;
}

#else

static auto _Get_known_files(const fs::path& path, const std::wstring_view& extension_prefix)
{
	runtime_assert(extension_prefix.starts_with('.'), "Incorrect extension prefix");
	std::vector<std::string> out;

	for (const auto& entry : fs::directory_iterator(path))
	{
		if (!entry.is_regular_file( ))
			continue;

		const auto& str = entry.path( ).native( );

		const auto filename = _Parse_filename<false>(str);
		const auto extension = _Parse_extension(filename);

		if (!extension.starts_with(extension_prefix))
			continue;

		out.push_back({filename.begin( ), filename.end( )});
	}

	return out;
}

static auto _Get_includes(std::string_view type, bool detect_duplicates)
{
	using include_name = std::pair<std::string, bool>;
	std::vector<include_name> result;
	nstd::unordered_set<std::string_view> results_used; //skip stuff like array<array<X,....

	const auto pat = std::regex("[\\w:*]+");
	const auto start = std::regex_iterator(type.begin( ), type.end( ), pat);
	const auto end = decltype(start)();

	/*constexpr auto subrange_to_string_view = []<class T>(T && srng)-> std::string_view
	{
		return { std::addressof(*srng.begin()), static_cast<size_t>(std::ranges::distance(srng)) };
	};*/

	for (const auto& match : std::ranges::subrange(start, end))
	{
#if 0
		_NODISCARD string_type str(size_type _Sub = 0) const
		{
			return string_type((*this)[_Sub]);
		}
#endif
		auto& match0 = match[0];
#if 0
		_NODISCARD string_type str( ) const
		{ // convert matched text to string
			const _Mybase _Range(_Effective_range( ));
			return string_type(_Range.first, _Range.second);
		}
#endif
		std::string_view str = {match0.first, match0.second}; //match.str()

		//-----

		if (detect_duplicates && !results_used.emplace(str).second)
			continue;

		if (str.find(':') != str.npos)
		{
			constexpr std::string_view std_namespace = "std::";
			constexpr std::string_view cheat_namespace = "cheat::";

			auto global = !str.starts_with(cheat_namespace);

			if (global)
			{
				if (str.starts_with(std_namespace))
					str.remove_prefix(std_namespace.size( ));
			}

			std::string include;
#if 0
			//correct, but currently incompatible because of broken compiler
			for (const auto subrng : str | std::views::split("::"))
			{
				if (!include.empty( ))
					include += '/';
				include += subrange_to_string_view(subrng);
			}
#else
			runtime_assert("TEST ME");
			size_t pos = 0;
			const auto ptr = str._Unchecked_begin( );
			for (;;)
			{
				const auto pos2 = str.find("::", pos);
				if (pos2 == str.npos)
					break;
				const auto substr = std::string_view(ptr + pos, ptr + pos2);
				if (!include.empty( ))
					include += '/';
				include += substr;
				pos = pos2 + 2;
			}
#endif

			result.emplace_back(std::move(include), global);
		}
		else if (str.ends_with("_t"))
		{
			result.emplace_back("cstdint", true);
		}
		}

	return result;
	}

struct generated_includes
{
	std::vector<std::string> local, global;
};

static auto _Generate_includes_from_types(const netvars_impl::netvars_storage& storage)
{
	using namespace std::string_view_literals;
	generated_includes includes;
	const fs::path csgosdk_dir = STRINGIZE_PATH(CHEAT_CSGOSDK_DIR);

	nstd::unordered_set<std::string> processed;
	const auto known_files = _Get_known_files(csgosdk_dir, L".h");

	for (auto& [netvar_name, netvar_data] : storage.items( ))
	{
		const std::string_view netvar_type = netvar_data.find("type"sv)->get_ref<const std::string&>( );

		//netvar type already processed
		if (!processed.emplace(netvar_type).second)
			continue;
		if (netvar_type.find(':') == netvar_type.npos && !netvar_type.ends_with("_t"))
			continue;

		const auto templates_count = std::ranges::count(netvar_type, '<');
		auto types_found = _Get_includes(netvar_type, templates_count > 1);
		for (auto& [incl, global] : types_found)
		{
			if (templates_count > 0 && !processed.emplace(incl).second)
				continue;

			if (global)
			{
				includes.global.push_back(std::move(incl));
			}
			else
			{
				const auto incl_sv = _Parse_filename<false, std::string_view>(incl);
				for (const std::string_view file : known_files)
				{
					if (!file.starts_with(incl_sv))
						continue;
					const auto extension = file.substr(incl_sv.size( ));
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

namespace nstd
{
	template <typename T>
	struct hash<generated_file<T>>
	{
		size_t operator()(const generated_file<T>& f) const
		{
			return std::invoke(hash<T>( ), /*_Parse_filename*/f.str);
		}
	};
}

template <class StrType, typename GenFile = generated_file<StrType>, typename ViewT = typename GenFile::string_view_type>
static auto _Get_generated_files(const fs::path& folder, const ViewT& cpp, const ViewT& h)
{
	nstd::unordered_set<GenFile> storage;

	for (const auto& entry : fs::directory_iterator(folder))
	{
		const auto& str = entry.path( ).native( );
		const auto add_str = [&](const ViewT& postfix)
		{
			const auto& fstr = folder.native( );
			if constexpr (std::constructible_from<ViewT, std::wstring>)
			{
				return storage.emplace(str, fstr, postfix);
			}
			else
			{
				nstd::unistring<typename GenFile::value_type> fstr_correct = fstr;
				return storage.emplace(str, fstr_correct, postfix);
			}
		};

		using namespace nstd::enum_operators;
		if (str.ends_with(cpp))
			add_str(cpp).first->type |= generated_file_type::CPP;
		else if (str.ends_with(h))
			add_str(h).first->type |= generated_file_type::H;
	}

	return storage;
}
#endif

void netvars_impl::generate_classes(log_info info, netvars_storage& root_netvars_data, lazy::files_storage& lazy_storage)
{
	const fs::path generated_classes_dir = STRINGIZE_PATH(CHEAT_NETVARS_GENERATED_DIR);
	runtime_assert(std::_Is_slash(generated_classes_dir.native( ).back( )));

	constexpr std::string_view suffix_h = _STRINGIZE(CHEAT_NETVARS_GENERATED_HEADER_POSTFIX);
	constexpr std::string_view suffix_cpp = _STRINGIZE(CHEAT_NETVARS_GENERATED_SOURCE_POSTFIX);

	constexpr std::wstring_view wsuffix_h = _CRT_WIDE(_STRINGIZE(CHEAT_NETVARS_GENERATED_HEADER_POSTFIX));
	constexpr std::wstring_view wsuffix_cpp = _CRT_WIDE(_STRINGIZE(CHEAT_NETVARS_GENERATED_SOURCE_POSTFIX));

	nstd::mem::backup<netvars_storage> netvars_data_backup;

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

		netvars_data_backup = root_netvars_data;

		using generated_file_path = std::wstring;
		const auto generated_files = _Get_generated_files<generated_file_path>(generated_classes_dir, wsuffix_cpp, wsuffix_h);
		for (const auto& gen_file : generated_files)
		{
			const auto TYPE = gen_file.type;
			runtime_assert(TYPE != generated_file_type::UNSET);
			const auto& FILE = gen_file.str;

			const auto add_to_erase = [&](const std::wstring_view& postfix)
			{
				generated_file_path path;
				_Reserve_append(path, generated_classes_dir.native( ), FILE, postfix);
				lazy_storage.remove.emplace_back(std::move(path), false);
			};

			if (TYPE & (generated_file_type::H | generated_file_type::CPP))
			{
				const nstd::unistring<char> filechr = FILE;
				const std::string_view filechr_sv = {reinterpret_cast<const char*>(filechr._Unchecked_begin( )), filechr.size( )};
				const auto name_entry = root_netvars_data.find(filechr_sv);
				if (name_entry != root_netvars_data.end( ))
					continue;
				root_netvars_data.erase(name_entry);
			}
			else if (TYPE & generated_file_type::H)
			{
				add_to_erase(wsuffix_h);
			}
			else if (TYPE & generated_file_type::CPP)
			{
				add_to_erase(wsuffix_cpp);
			}
			else
			{
				runtime_assert("flags are broken");
			}
		}

		if (!root_netvars_data.empty( ) && (netvars_data_backup.get( ).size( ) != root_netvars_data.size( ) || !lazy_storage.remove.empty( )))
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
	lazy_storage.write.reserve(root_netvars_data.size( ) * 2);

	const auto make_file_writer = [&](const std::string_view& class_name, const std::string_view& suffix) -> lazy::file_writer
	{
		std::wstring tmp;
		_Reserve_append(tmp, class_name, suffix);
		return generated_classes_dir / std::move(tmp);
	};

	for (const auto& [CLASS_NAME, NETVARS] : root_netvars_data.items( ))
	{
#ifndef CHEAT_NETVARS_GENERATE_MODULES
		auto header = make_file_writer(CLASS_NAME, suffix_h);
#endif
		auto source = make_file_writer(CLASS_NAME, suffix_cpp);

#ifdef CHEAT_NETVARS_GENERATE_MODULES

		//todo: include std stuff

		source << "module cheat.csgo.structs." << CLASS_NAME << ':' << '\n';
		source << "import cheat.core.netvars;" << '\n'(2); //TEST NAME

#else
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

		const auto source_add_dynamic_includes = [&]
		{
			auto [local, global] = _Generate_includes_from_types(static_cast<const netvars_storage&>(NETVARS));
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
		};

		source_add_include(CLASS_NAME + ".h");
		source_add_include("cheat/netvars/netvars.h");
		source_add_include("nstd/address.h", true);
		source << '\n';

		source_add_dynamic_includes( );

		//source << "using cheat::csgo::" << CLASS_NAME << ';' << '\n';
		source << "using namespace cheat::csgo;" << '\n';
		source << '\n';
#endif

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
				stream << netvar_type << netvar_ret_char << ' ';
				if (!inside_class)
					stream << CLASS_NAME << "::";
				stream << NETVAR_NAME << "( )";
				if (inside_class)
					stream << ';';
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
					<< "netvars_impl::get( )->at"
					<< "(\"" << CLASS_NAME << ", " << '\"' << NETVAR_NAME << "\")"
#endif
					<< ";\n"
					<< '	'
					<< "auto addr = "
					<< nstd::type_name<nstd::address>( )
					<< "(this).add(offset);\n"

					<< "return addr." << (netvar_type_pointer ? "ptr" : "ref") << '<' << netvar_type << ">( );\n"
					<< "}\n";
			};

			write_fn_head(header, true);
			write_fn_head(source, false);
			write_fn_body(source);

#if 0
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

		lazy_storage.write.push_back(std::move(header));
		lazy_storage.write.push_back(std::move(source));
	}

	(void)netvars_data_backup;

#ifdef CHEAT_HAVE_CONSOLE
	std::string removed_msg;
	if (!lazy_storage.remove.empty( ))
	{
		if (lazy_storage.remove.size( ) == 1 && lazy_storage.remove[0].all( ))
			removed_msg = " Whole folder removed.";
		else
			removed_msg = std::format(" Removed {} files.", lazy_storage.remove.size( ));
	}
	std::string created_msg;
	if (!lazy_storage.write.empty( ))
	{
		created_msg = std::format(" Created {} files.", lazy_storage.write.size( ));
	}
	console::get( ).log(std::format("Netvars classes generation done.{}{}", removed_msg, created_msg));
#endif

	if (info == log_info::created)
	{
		//write all without waiting
		auto dummy = lazy::files_storage( );
		std::swap(lazy_storage, dummy);
	}
}
