#include "library_info.h"
#include "log.h"
#include "mem_scanner.h"
#include "tool/functional.h"
#include "tool/span.h"
#include "tool/string.h"
#include "tool/string_view.h"
#include "tool/vector.h"

#include <windows.h>
#include <intrin.h>
#include <winternl.h>

#include <cassert>

namespace fd
{
class rtti_descriptor_finder;
}

template <>
struct fmt::formatter<fd::rtti_descriptor_finder> : formatter<string_view>
{
    auto format(fd::rtti_descriptor_finder const &finder, format_context &ctx) const -> format_context::iterator;
};

namespace fd
{
using placeholders::_1;

class cast_helper
{
    void *from_;

  public:
    cast_helper(void *from)
        : from_(from)
    {
    }

    template <typename Q>
    operator Q() const requires(sizeof(Q) == sizeof(void *))
    {
        return static_cast<Q>(from_);
    }
};

static system_string_view library_path(system_library_entry entry)
{
    wstring_view path;
    auto &ustr = entry->FullDllName;
    if (ustr.Buffer)
        path = {ustr.Buffer, ustr.Length / sizeof(WCHAR)};
    return path;
}

static system_string_view library_name(system_string_view path)
{
    auto name_start = path.rfind('\\');
    assert(name_start != path.npos);
    return path.substr(name_start + 1);
}

static system_string_view library_name(system_library_entry entry)
{
    return library_name(library_path(entry));
}

static system_string_view library_name(system_library_entry entry, size_t limit)
{
    using l = std::numeric_limits<size_t>;
    assert(limit != l::min() && limit != l::max());
    auto path = library_path(entry);
    assert(path.length() > limit);
    path.remove_prefix(path.length() - (limit + 1));
    auto name_start = path.rfind('\\');
    system_string_view ret;
    if (name_start != path.npos)
    {
        ret = path.substr(name_start + 1);
        assert(!ret.empty());
    }
    return ret;
}

static bool library_has_name(system_library_entry entry, system_string_view name)
{
    auto path = library_path(entry);
    path.remove_prefix(path.length() - (name.length() + 1));
    return path.starts_with('\\') && path.ends_with(name);
}

template <std::invocable<system_library_entry> Filter>
static system_library_entry find_library(Filter filter) noexcept
{
#ifdef _WIN64
    auto mem = NtCurrentTeb();
    auto ldr = mem->ProcessEnvironmentBlock->Ldr;
#else
    auto mem = reinterpret_cast<PEB *>(__readfsdword(0x30));
    auto ldr = mem->Ldr;
#endif
    auto root_list = &ldr->InMemoryOrderModuleList;

    for (auto entry = root_list->Flink; entry != root_list; entry = entry->Flink)
    {
        auto table = CONTAINING_RECORD(entry, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
        if (filter(table))
            return table;
    }
    return nullptr;
}

static system_library_entry find_library(void *base_address)
{
    return find_library(_1->*&LDR_DATA_TABLE_ENTRY::DllBase == base_address);
}

static system_library_entry find_library(system_string_view name)
{
    return find_library(bind(library_has_name, _1, name));
}

system_library::system_library(system_string_view name)
    : entry_(find_library(name))
{
}

system_string_view system_library::name() const
{
    return library_name(entry_);
}

system_string_view system_library::path() const
{
    return library_path(entry_);
}

static IMAGE_DOS_HEADER *get_dos(system_library_entry entry)
{
    auto dos = static_cast<IMAGE_DOS_HEADER *>(entry->DllBase);
    // check for invalid DOS / DOS signature.
    assert(dos->e_magic == IMAGE_DOS_SIGNATURE /* 'MZ' */);
    return dos;
}

static IMAGE_NT_HEADERS *get_nt(IMAGE_DOS_HEADER *dos)
{
    auto nt = reinterpret_cast<IMAGE_NT_HEADERS *>(reinterpret_cast<uintptr_t>(dos) + dos->e_lfanew);
    // check for invalid NT / NT signature.
    assert(nt->Signature == IMAGE_NT_SIGNATURE /* 'PE\0\0' */);
    return nt;
}

static IMAGE_NT_HEADERS *get_nt(system_library_entry entry)
{
    assert(_ReturnAddress() != get_dos);
    return get_nt(get_dos(entry));
}

static auto memory_range(IMAGE_NT_HEADERS *nt)
{
    auto begin  = reinterpret_cast<uint8_t *>(nt->OptionalHeader.ImageBase);
    auto length = nt->OptionalHeader.SizeOfImage;
    return std::pair{begin, begin + length};
}

static auto memory_range(system_library_entry entry)
{
    return memory_range(get_nt(entry));
}

span<uint8_t> system_library::memory() const
{
    auto [begin, end] = memory_range(entry_);
    return {begin, end};
}

struct export_data
{
    union
    {
        IMAGE_DOS_HEADER *dos_header;
        uint8_t *dos;
    };

    union
    {
        IMAGE_EXPORT_DIRECTORY *export_dir;
        uint8_t *virtual_addr_start;
    };

    uint8_t *virtual_addr_end;

    uint32_t *names;
    uint32_t *funcs;
    uint16_t *ords;

    IMAGE_EXPORT_DIRECTORY *operator->() const
    {
        return export_dir;
    }

    export_data(IMAGE_DOS_HEADER *dos_header, IMAGE_NT_HEADERS *nt_header)
        : dos_header(dos_header)
    {
        auto &data_dir = nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];

        virtual_addr_start = cast_helper(dos + data_dir.VirtualAddress);
        // not sure
        virtual_addr_end   = cast_helper(virtual_addr_start + data_dir.Size);

        names = cast_helper(dos + export_dir->AddressOfNames);
        funcs = cast_helper(dos + export_dir->AddressOfFunctions);
        ords  = cast_helper(dos + export_dir->AddressOfNameOrdinals);
    }
};

class export_view
{
    size_t offset_;
    export_data *data_;

  public:
    export_view(size_t offset, export_data *data)
        : offset_(offset)
        , data_(data)
    {
    }

    char const *name() const
    {
        return reinterpret_cast<char const *>(data_->dos + data_->names[offset_]);
    }

    void *function() const
    {
        void *tmp = data_->dos + data_->funcs[data_->ords[offset_]];
        if (tmp < data_->virtual_addr_start || tmp >= data_->virtual_addr_end)
            return tmp;

        // todo:resolve fwd export
        assert(0 && "Forwarded export detected");
        return nullptr;

#if 0
		// get forwarder std::string.
		std::string_view fwd_str = export_ptr.get<const char*>( );

		// forwarders have a period as the delimiter.
		auto delim = fwd_str.find_last_of('.');
		if(delim == fwd_str.npos)
			continue;

		using namespace string_view_literals;
		// get forwarder mod name.
		const info_string::fixed_type fwd_module_str = nstd::append<std::wstring>(fwd_str.substr(0, delim), L".dll"sv);

		// get real export ptr ( recursively ).
		auto target_module = std::ranges::find_if(*all_modules, [&](const info& i)
		{
			return i.name == fwd_module_str;
		});
		if(target_module == all_modules->end( ))
			continue;

		// get forwarder export name.
		auto fwd_export_str = fwd_str.substr(delim + 1);

		try
		{
			auto& exports = target_module->exports( );
			auto fwd_export_ptr = exports.at(fwd_export_str);

			this->emplace(export_name, fwd_export_ptr);
		}
		catch(std::exception)
		{
		}
#endif
    }
};

void *system_library::function(string_view name) const
{
    auto dos = get_dos(entry_);
    auto nt  = get_nt(dos);

    auto edata = export_data(dos, nt);

    size_t last_offset = std::min(edata->NumberOfNames, edata->NumberOfFunctions);
    for (size_t offset = 0; offset != last_offset; ++offset)
    {
        export_view view(offset, &edata);
        if (view.name() == name)
            return view.function();
    }
    return nullptr;
}

uint8_t *system_library::pattern(string_view pattern) const
{
    auto [begin, end] = memory_range(entry_);
    return static_cast<uint8_t *>(find_pattern(begin, end, pattern.data(), pattern.length()));
}

static system_section_header find_section(IMAGE_NT_HEADERS *nt, char const *name, size_t name_length)
{
    auto begin = IMAGE_FIRST_SECTION(nt);
    auto end   = begin + nt->FileHeader.NumberOfSections;

    for (; begin != end; ++begin)
    {
        if (begin->Name[name_length] == '\0' && memcmp(begin->Name, name, name_length) == 0)
            return begin;
    }

    return nullptr;
}

template <size_t S>
static system_section_header find_section(IMAGE_NT_HEADERS *nt, char const (&name)[S])
{
    return find_section(nt, name, S - 1);
}

system_section_header system_library::section(string_view name) const
{
    return find_section(get_nt(entry_), name.data(), name.length());
}

static auto memory_range(IMAGE_DOS_HEADER *dos, system_section_header header)
{
    auto begin  = reinterpret_cast<uint8_t *>(dos) + header->VirtualAddress;
    auto length = header->SizeOfRawData;
    return std::pair{begin, begin + length};
}

class rtti_descriptor_finder
{
    string_view class_name_;
    void *found_;

  public:
    rtti_descriptor_finder(IMAGE_NT_HEADERS *nt, string_view class_name)
        : class_name_(class_name)
    {
        auto do_find = [this, nt]<typename P>(P symbol) {
            static_vector<P, 64> full_descriptor;

            auto writer = [&]<typename A>(A const &arg) {
                if constexpr (std::convertible_to<A, P>)
                    full_descriptor.push_back(arg);
                else
                    std::copy(
                        std::begin(arg),
                        std::end(arg) - std::is_bounded_array_v<A>, //
                        std::back_inserter(full_descriptor));
            };

            writer(".?A");
            writer(symbol);
            writer(class_name_);
            writer("@@");

            auto [begin, end] = memory_range(nt);
            if constexpr (std::same_as<P, special_pattern_tag>)
                found_ = find_pattern(begin, end, full_descriptor.data(), full_descriptor.size());
            else
                found_ = find_bytes(begin, end, full_descriptor.data(), full_descriptor.size());
        };

        if (auto space = class_name.find(' '); space == class_name.npos)
            do_find(special_pattern_gap);
        else
        {
            auto info   = class_name.substr(0, space - 1);
            class_name_ = class_name.substr(space);

            if (info == "struct")
                do_find('U');
            else if (info == "class")
                do_find('V');
            else
                std::unreachable();
        }
    }

    operator void *() const
    {
        return found_;
    }

    void *get() const
    {
        return found_;
    }

    string_view name() const
    {
        return class_name_;
    }

    string_view raw_name() const
    {
        return {static_cast<char *>(found_), 3 /*.?A*/ + 1 /*U\V*/ + class_name_.length() + 2 /*@@*/};
    }
};

void *system_library::rtti_descriptor(string_view class_name) const
{
    rtti_descriptor_finder finder(get_nt(entry_), class_name);

    if (finder)
        log("rtti descriptor {} found", finder);
    else
        log("rtti descriptor {} not found", finder);

    return finder;
}

void *system_library::vtable(string_view name) const
{
    auto dos = get_dos(entry_);
    auto nt  = get_nt(dos);

    rtti_descriptor_finder rtti_finder(nt, name);

    // get rtti type descriptor
    auto type_descriptor = reinterpret_cast<uintptr_t>(rtti_finder.get());
    // we're doing - 0x8 here, because the location of the rtti typedescriptor is 0x8 bytes before the std::string
    type_descriptor -= sizeof(uintptr_t) * 2;

    // dos + section->VirtualAddress, section->SizeOfRawData

    auto [rdata_begin, rdata_end] = memory_range(dos, find_section(nt, ".rdata"));
    auto [text_begin, text_end]   = memory_range(dos, find_section(nt, ".text"));

    auto addr1          = find_xref(rdata_begin, rdata_end, type_descriptor, [&](uintptr_t &xref, auto *stop_token) {
        // get offset of vtable in complete class, 0 means it's the class we need, and not
        // some class it inherits from
        auto offset = *reinterpret_cast<uint32_t *>(xref - 0x8);
        if (offset == 0)
            stop_token->stop();
        return true;
    });
    auto object_locator = addr1 - 0xC;
    auto addr2          = find_xref(rdata_begin, rdata_end, object_locator) + 0x4;
    // check is valid offset
    assert(addr2 != 0x4);
    auto found = reinterpret_cast<void *>(find_xref(text_begin, text_end, addr2));

    if (found)
        log("vtable {} found", rtti_finder);
    else
        log("vtable {} NOT found", rtti_finder);

    return found;
}

} // namespace fd

auto fmt::formatter<fd::rtti_descriptor_finder>::format(
    fd::rtti_descriptor_finder const &finder,
    format_context &ctx) const -> format_context::iterator
{
    if (!finder)
        return formatter<string_view>::format(unwrap(finder.name()), ctx);

    fd::small_vector<char, 64> buff;
    format_to(std::back_inserter(buff), "{} ({})", finder.name(), finder.raw_name());
    return formatter<string_view>::format({buff.data(), buff.size()}, ctx);
}

#ifdef _DEBUG
auto fmt::formatter<fd::system_library::bound_name>::format(
    fd::system_library::bound_name binder,
    format_context &ctx) const -> format_context::iterator
{
    auto name = binder();
    fd::small_vector<char, 32> buff;
    buff.assign(name.begin(), name.end());
    return formatter<string_view>::format({buff.data(), buff.size()}, ctx);
}

namespace fd
{
static thread_local small_vector<char, 128> fmt_args_buff;

fmt::string_view system_library::merge_fmt_args(fmt::string_view fmt) const
{
    auto n = name();

    fmt_args_buff.resize(n.length() + 2 + fmt.size());

    auto begin = fmt_args_buff.data();
    auto it    = std::copy(n.begin(), n.end(), begin);
    *it++      = ':';
    *it++      = ' ';
    it         = std::copy(fmt.begin(), fmt.end(), it);
    return {begin, static_cast<size_t>(std::distance(begin, it))};
}
} // namespace fd

#endif