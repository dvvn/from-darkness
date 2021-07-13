#include "module info.h"

using namespace cheat::utl;
using namespace detail;

void module_info::Fix_vtables_cache_sections_( )
{
	//members__.vtables.sections__ = addressof(members__.sections);
	members__.vtables.set_sections(addressof(members__.sections));
}

module_info::module_info(LDR_DATA_TABLE_ENTRY* ldr_entry, IMAGE_DOS_HEADER* dos, IMAGE_NT_HEADERS* nt)
{
	BOOST_ASSERT(ldr_entry != nullptr);
	BOOST_ASSERT(dos != nullptr);
	BOOST_ASSERT(nt != nullptr);

	//manual_handle_ = winapi::module_handle(handle);

	info__.ldr_entry = ldr_entry;
	info__.dos = dos;
	info__.nt = nt;

	const auto base = this->base( );

	members__.sections = {base, nt};
	members__.exports = {base, nt};
	members__.vtables = {base, image_size( ), nt, addressof(members__.sections)};

#ifdef _DEBUG
	//preload for better debugging

	sections( ).load( );
	if (name_contains_unicode( ))
		members__.name.clear( );
	else
	{
		name( );
		members__.name_wide.clear( );
	}
#endif
}

module_info::module_info(module_info&& other) noexcept
{
	*this = move(other);
}

module_info& module_info::operator=(module_info&& other) noexcept
{
	info__ = move(other.info__);
	members__ = move(other.members__);

	Fix_vtables_cache_sections_( );

	return *this;
}

#if 0
template <typename Chr, typename Traits, typename ...Next>
std::basic_string<Chr, Traits> prefix_adder(const std::basic_string_view<Chr, Traits>& first, Next&&...other)
{
    using view_t = std::remove_cvref_t<decltype(first)>;
    const auto cache = std::array/*<view_t, sizeof...(Next) + 1>*/{ first, view_t(other)... };

    size_t ideal_size = 1;
    for (const view_t& text : cache)
        ideal_size += text.size();

    std::basic_string<Chr, Traits> result;
    result.reserve(ideal_size);

    for (const view_t& text : cache)
        result += text;

    return result;

    //return cheat::utl::combine_strings(_Add_prefix_impl(FWD(args))...);File_on_disc_impl_
}
#endif

#if 0

auto module_info::File_on_disc_impl_(const wstring_view& module_name, IMAGE_NT_HEADERS*      nt_header, const filesystem::path& folder,
									 const wstring_view& file_extension, const wstring_view& file_name_postfix) -> filesystem::path
{
	const auto check_sum = nt_header->OptionalHeader.CheckSum;
	if (check_sum == 0)
	{
		BOOST_ASSERT("Unable to create file path because checksum is null!");
		return { };
	}

	auto file_name = (to_string(check_sum));
	if (!file_extension.empty( ))
	{
		if (!file_extension.starts_with('.'))
			file_name += '.';
		file_name.append(file_extension.begin( ), file_extension.end( ));
	}

	//----------------------

	wstring current_folder;
	if (file_name_postfix.empty( ))
	{
		current_folder = fmt::format(L"_{}", module_name);
	}
	else
	{
		current_folder = fmt::format(L"_{}_{}", module_name, file_name_postfix);
	}

	//----------------------

	return folder / current_folder / file_name;
}

#endif

address module_info::base( ) const
{
	return info__.ldr_entry->DllBase;
}

memory_block module_info::mem_block( ) const
{
	return {base( ), image_size( )};
}

// ReSharper disable CppInconsistentNaming

IMAGE_DOS_HEADER* module_info::DOS( ) const
{
	return info__.dos;
}

IMAGE_NT_HEADERS* module_info::NT( ) const
{
	return info__.nt;
}

DWORD module_info::check_sum( ) const
{
	return NT( )->OptionalHeader.CheckSum;
}

// ReSharper restore CppInconsistentNaming

DWORD module_info::code_size( ) const
{
	return NT( )->OptionalHeader.SizeOfCode;
}

DWORD module_info::image_size( ) const
{
	return NT( )->OptionalHeader.SizeOfImage;
}

module_info::raw_name_type module_info::work_dir( ) const
{
	auto path_to = full_path( );
	path_to.remove_suffix(raw_name( ).size( ));
	return path_to;
}

module_info::raw_name_type module_info::full_path( ) const
{
	return {info__.ldr_entry->FullDllName.Buffer, info__.ldr_entry->FullDllName.Length / sizeof(raw_name_value_type)};
}

module_info::raw_name_type module_info::raw_name( ) const
{
	const auto path = full_path( );
	return path.substr(path.find_last_of('\\') + 1);
}

const string& module_info::name( )
{
	if (members__.name.empty( ))
	{
		const auto raw = this->raw_name( );
		string     out;
		out.reserve(raw.size( ));

		for (const auto wchr: raw)
		{
			if constexpr (sizeof(raw_name_value_type) == sizeof(string::value_type))
			{
				out += tolower(wchr);
			}
			else
			{
				if (const char chr = wchr; chr == wchr)
					out += tolower(chr);
				else
				{
					BOOST_ASSERT("Unable to convert wide character!");
					out.clear( );
					out += '\0';
					break;
				}
			}
		}

		members__.name = move(out);
	}

	return members__.name;
};

const string& module_info::name( ) const
{
	return members__.name;
}

const wstring& module_info::name_wide( )
{
	if (members__.name_wide.empty( ))
	{
		static_assert(sizeof(raw_name_value_type) <= sizeof(wstring::value_type));
		const auto raw = this->raw_name( );
		wstring    out;
		out.reserve(raw.size( ));
		for (const auto wchr: raw)
			out += towlower(wchr);

		members__.name_wide = move(out);
	}
	return members__.name_wide;
}

const wstring& module_info::name_wide( ) const
{
	return members__.name_wide;
}

bool module_info::name_contains_unicode( )
{
	auto& wname = this->name_wide( );
	return IsTextUnicode(wname.c_str( ), wname.size( ) * sizeof(wstring::value_type), nullptr);
}

bool module_info::name_contains_unicode( ) const
{
	auto& wname = this->name_wide( );
	BOOST_ASSERT_MSG(!wname.empty(), "Wide name unset!");
	return IsTextUnicode(wname.c_str( ), wname.size( ) * sizeof(wstring::value_type), nullptr);
}

sections_storage& module_info::sections( )
{
	return members__.sections;
}

const sections_storage& module_info::sections( ) const
{
	return members__.sections;
}

exports_storage& module_info::exports( )
{
	return members__.exports;
}

const exports_storage& module_info::exports( ) const
{
	return members__.exports;
}

vtables_storage& module_info::vtables( )
{
	return members__.vtables;
}

const vtables_storage& module_info::vtables( ) const
{
	return members__.vtables;
}

struct headers_info
{
	PIMAGE_DOS_HEADER dos;
	PIMAGE_NT_HEADERS nt;
};

optional<headers_info> _Get_file_headers(const address& base)
{
	const auto dos = base.raw<IMAGE_DOS_HEADER>( );

	// check for invalid DOS / DOS signature.
	if (!dos || dos->e_magic != IMAGE_DOS_SIGNATURE /* 'MZ' */)
		return { };

	// get NT headers.
	const auto nt = address(dos).add(dos->e_lfanew).raw<IMAGE_NT_HEADERS>( );

	// check for invalid NT / NT signature.
	if (!nt || nt->Signature != IMAGE_NT_SIGNATURE /* 'PE\0\0' */)
		return { };

	headers_info result;

	// set out dos and nt.
	result.dos = dos;
	result.nt = nt;

	return result;
}

modules_storage_container _Get_all_modules( )
{
	// TEB->ProcessEnvironmentBlock.
#ifdef UTILS_X64
    const auto mem = NtCurrentTeb();
    BOOST_ASSERT_MSG(mem != nullptr, "Teb not found");
#else
	const auto mem = reinterpret_cast<PPEB>(__readfsdword(0x30));
	BOOST_ASSERT_MSG(mem != nullptr, "Peb not found");
#endif

#ifdef UTILS_X64
    auto ldr = mem->ProcessEnvironmentBlock->Ldr;
#else
	auto ldr = mem->Ldr;
#endif

	auto container = modules_storage_container( );

	// get module linked list.
	const auto list = &ldr->InMemoryOrderModuleList;
	// iterate linked list.
	for (auto it = list->Flink; it != list; it = it->Flink)
	{
		// get current entry.
		const auto ldr_entry = CONTAINING_RECORD(it, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
		if (!ldr_entry)
			continue;

		// get file headers and ensure it's a valid PE file.
		const auto headers = _Get_file_headers(ldr_entry->DllBase);
		if (!headers.has_value( ))
			continue;

		// push module to out container.
		container.emplace_back(ldr_entry, headers->dos, headers->nt);
	}

	//all internal functions tested only on x86
	//UPDATE: all works, except vtables finder

	container.shrink_to_fit( );
	return (container);
}

//module_info_loaded::module_info_loaded(const boost::filesystem::path& p, DWORD flags):
//    handle_(LoadLibraryExW(p.c_str( ), nullptr, flags))
//{
//    BOOST_ASSERT_MSG(handle_!=nullptr, "Unable to load module");
//
//
//    auto all_modules=get_all_modules()
//
//}

modules_storage::modules_storage( )
{
	this->update( );
}

modules_storage& modules_storage::update(bool force)
{
	if (force || storage__.empty( ))
	{
		current_cached__ = nullptr;
		storage__ = _Get_all_modules( );
	}
	return *this;
}

static HMODULE _Get_current_module_handle( )
{
	MEMORY_BASIC_INFORMATION info;
	//todo: is this is dll, try to load this fuction from inside
	const size_t len = VirtualQueryEx(GetCurrentProcess( ), _Get_current_module_handle, &info, sizeof(MEMORY_BASIC_INFORMATION));
	BOOST_ASSERT_MSG(len == sizeof(info), "Wrong size");
	return static_cast<HMODULE>(info.AllocationBase);
}

module_info& modules_storage::current( )
{
	if (current_cached__ != nullptr)
		return *current_cached__;

	const address handle = _Get_current_module_handle( );

	for (auto& info: storage__)
	{
		if (info.base( ) == handle)
		{
			current_cached__ = addressof(info);
			return current( );
		}
	}

	BOOST_ASSERT("Unable to find current module");
	return *static_cast<module_info*>(nullptr); //force error
}

const module_info& modules_storage::current( ) const
{
	return *current_cached__;
}

module_info& modules_storage::owner( )
{
	return storage__.front( );
}

const module_info& modules_storage::owner( ) const
{
	return storage__.front( );
}

#if 0
module_info& modules_storage::load(const boost::filesystem::path& from, DWORD flags)
{
    auto handle = LoadLibraryExW(from.c_str(), nullptr, flags);
    BOOST_ASSERT_MSG(handle != nullptr, "Unable to load module");

    this->update(true);

    auto added = boost::range::find_if(storage_, [&](module_info& info) { return info.base() == handle; });
    BOOST_ASSERT_MSG(added != storage_.end(), "Unable to find added module!");

    auto& handle_closer = added->manual_handle_;
    BOOST_ASSERT_MSG(handle_closer.get() != handle, "Handle already set!");

    added->manual_handle_ = winapi::module_handle(handle);
    return *added;
}
#endif

modules_storage_container& modules_storage::all(bool update)
{
	this->update(update);
	return storage__;
}

const modules_storage_container& modules_storage::all( ) const
{
	return storage__;
}

module_info* modules_storage::find(const string_view& name)
{
	for (auto& entry: storage__)
	{
		if (entry.name( ) == name)
			return addressof(entry);
	}
	return nullptr;
}

module_info* modules_storage::find(const wstring_view& name)
{
	for (auto& entry: storage__)
	{
		if (entry.name_wide( ) == name)
			return addressof(entry);
	}
	return nullptr;
}

void modules_storage::Empty_assert( ) const
{
	BOOST_ASSERT_MSG(!storage__.empty(), "Storage is empty!");
}
