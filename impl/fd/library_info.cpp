#include "library_info.h"
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
        auto view = export_view(offset, &edata);
        if ((view.name() == name))
            return view.function();
    }
    return nullptr;
}

void *system_library::pattern(string_view pattern) const
{
    auto [begin, end] = memory_range(entry_);
    return find_pattern(begin, end, pattern.data(), pattern.length());
}

static system_section_header find_section(IMAGE_NT_HEADERS *nt, string_view name)
{
    auto begin = IMAGE_FIRST_SECTION(nt);
    auto end   = begin + (nt->FileHeader.NumberOfSections);

    for (; begin != end; ++begin)
    {
        if (reinterpret_cast<char *>(begin->Name) == name)
            return begin;
    }

    return nullptr;
}

system_section_header system_library::section(string_view name) const
{
    return find_section(get_nt(entry_), name);
}

static auto memory_range(IMAGE_DOS_HEADER *dos, system_section_header header)
{
    auto begin  = reinterpret_cast<uint8_t *>(dos) + header->VirtualAddress;
    auto length = header->SizeOfRawData;
    return std::pair{begin, begin + length};
}

static void *find_rtti_descriptor(IMAGE_NT_HEADERS *nt, string_view class_name)
{
    auto make_sample = []<typename T, typename... Args>(T arg1, Args... args) {
        using std::ranges::range;
        using std::ranges::range_value_t;

        auto prepare_buffer = [] {
            if constexpr (range<T>)
                return static_vector<range_value_t<T>, 64>();
            else
                return static_vector<std::decay_t<T>, 64>();
        };

        auto buffer = prepare_buffer();
        auto writer = [&]<typename A>(A const &arg) {
            if constexpr (range<A>)
                std::copy(std::begin(arg), std::end(arg) - std::is_bounded_array_v<A>, std::back_inserter(buffer));
            else
                buffer.push_back(arg);
        };
        writer(".?A");
        writer(arg1);
        (writer(args), ...);
        writer("@@");
        return buffer;
    };

    auto [begin, end] = memory_range(nt);

    auto space = class_name.find(' ');
    if (space != class_name.npos)
    {
        char type_info;
        auto type_name = class_name.substr(0, space - 1);
        if (type_name == "struct")
            type_info = 'U';
        else if (type_name == "class")
            type_info = 'V';
        else
            std::unreachable();
        auto sample = make_sample(type_info, class_name.substr(space));
        return find_bytes(begin, end, sample.data(), sample.size());
    }

    auto sample = make_sample(special_pattern_gap, class_name);
    return find_pattern(begin, end, sample.data(), sample.size());
}

void *system_library::rtti_descriptor(string_view class_name) const
{
    return find_rtti_descriptor(get_nt(entry_), class_name);
}

void *system_library::vtable(string_view name) const
{
    auto dos = get_dos(entry_);
    auto nt  = get_nt(dos);

    // get rtti type descriptor
    auto type_descriptor = reinterpret_cast<uintptr_t>(find_rtti_descriptor(nt, name));
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
    return reinterpret_cast<void *>(find_xref(text_begin, text_end, addr2));
}
}