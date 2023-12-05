#pragma once

#include "container/span.h"
#include "functional/cast.h"
#include "memory/xref.h"
#include "pattern/make.h"
#include "string/static.h"
#include "string/view.h"
#include "pattern.h"

#include <windows.h>
#include <winternl.h>

#include <algorithm>
#include <cassert>

#undef interface

// ReSharper disable CppInconsistentNaming

// see https://www.vergiliusproject.com/kernels/x64/Windows%2011/22H2%20(2022%20Update)/_LDR_DATA_TABLE_ENTRY
typedef struct _LDR_DATA_TABLE_ENTRY_FULL
{
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;

    union
    {
        PVOID DllBase;
        PIMAGE_DOS_HEADER DosHeader;
        ULONG_PTR DllBaseAddress;
    };

    PVOID EntryPoint;

    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
} LDR_DATA_TABLE_ENTRY_FULL, *PLDR_DATA_TABLE_ENTRY_FULL;

// ReSharper restore CppInconsistentNaming

inline wchar_t* data(UNICODE_STRING const& ustr)
{
    return ustr.Buffer;
}

inline USHORT size(UNICODE_STRING const& ustr)
{
    return ustr.Length / sizeof(wchar_t);
}

inline wchar_t* begin(UNICODE_STRING const& ustr)
{
    return ustr.Buffer;
}

inline wchar_t* end(UNICODE_STRING const& ustr)
{
    return ustr.Buffer + size(ustr);
}

inline bool operator!(UNICODE_STRING const& ustr)
{
    return ustr.Buffer == nullptr;
}

namespace fd::native
{
class interface_register;
}

namespace fd
{
union library_base_address1
{
    PVOID DllBase;
    PIMAGE_DOS_HEADER DosHeader;
    ULONG_PTR DllBaseAddress;

    operator PVOID() const
    {
        return DllBase;
    }

    operator PIMAGE_DOS_HEADER() const
    {
        return DosHeader;
    }

    IMAGE_DOS_HEADER* operator->() const
    {
        return DosHeader;
    }

    void* operator+(ULONG_PTR const other) const
    {
        return reinterpret_cast<void*>(DllBaseAddress + other);
    }
};

class library_info
{
    union
    {
        LDR_DATA_TABLE_ENTRY_FULL* entry_full_;
        LDR_DATA_TABLE_ENTRY* entry_;
    };

    static LIST_ENTRY* module_list()
    {
#ifdef _WIN64
        auto const mem = reinterpret_cast<TEB*>(__readgsqword(FIELD_OFFSET(NT_TIB64, Self)));
        auto const ldr = mem->ProcessEnvironmentBlock->Ldr;
#else
        auto const mem = reinterpret_cast<PEB*>(__readfsdword(FIELD_OFFSET(NT_TIB32, Self)));
        auto const ldr = mem->Ldr;
#endif

        return &ldr->InMemoryOrderModuleList;
    }

    static LDR_DATA_TABLE_ENTRY_FULL* ldr_table(LIST_ENTRY* entry)
    {
        return CONTAINING_RECORD(entry, LDR_DATA_TABLE_ENTRY_FULL, InMemoryOrderLinks);
    }

  public:
    struct extension
    {
        static constexpr static_wstring dll = L".dll";
        static constexpr static_wstring exe = L".exe";
    };

    struct section_string : string_view
    {
    };

    struct section
    {
        static constexpr section_string rdata{".rdata"};
        static constexpr section_string text{".text"};
    };

    library_info(wstring_view const name)
    {
        auto const name_first = ubegin(name);
        auto const name_last  = uend(name);

        auto const root_list = module_list();
        for (auto list_entry = root_list->Flink; list_entry != root_list; list_entry = list_entry->Flink)
        {
            auto const entry = ldr_table(list_entry);
            if (!entry->BaseDllName)
                continue;
            if (!std::equal(name_first, name_last, begin(entry->BaseDllName), end(entry->BaseDllName)))
                continue;

            entry_full_ = entry;
            return;
        }

        entry_full_ = nullptr;
    }

    library_info(wstring_view const name, wstring_view const ext)
    {
        using std::equal;
        using std::size;

        auto const name_length      = size(name);
        auto const ext_length       = size(ext);
        auto const full_name_length = name_length + ext_length;

        auto const name_first = ubegin(name);
        auto const name_last  = uend(name);

        auto const ext_first = ubegin(ext);
        auto const ext_last  = uend(ext);

        auto const root_list = module_list();
        for (auto list_entry = root_list->Flink; list_entry != root_list; list_entry = list_entry->Flink)
        {
            auto const entry = ldr_table(list_entry);
            if (!entry->BaseDllName)
                continue;
            if (full_name_length != size(entry->BaseDllName))
                continue;
            auto const entry_first = ubegin(entry->BaseDllName);
            if (!equal(ext_first, ext_last, entry_first + name_length))
                continue;
            if (!equal(name_first, name_last, entry_first))
                continue;

            entry_full_ = entry;
            return;
        }

        entry_full_ = nullptr;
    }

    template <std::derived_from<library_info> Other>
    library_info(Other other)
        : entry_{other.entry_}
    {
    }

    explicit operator bool() const
    {
        return entry_ != nullptr;
    }

  private:
    IMAGE_DOS_HEADER* dos_header() const
    {
        auto const dos = entry_full_->DosHeader;
        assert(dos->e_magic == IMAGE_DOS_SIGNATURE);
        return dos;
    }

    IMAGE_NT_HEADERS* nt_header() const
    {
        auto const dos = dos_header();
        auto const nt  = unsafe_cast<IMAGE_NT_HEADERS*>(entry_full_->DllBaseAddress + dos->e_lfanew);
        assert(nt->Signature == IMAGE_NT_SIGNATURE);
        return nt;
    }

    /*library_base_address base() const
    {
        return {entry_full_->DllBase};
    }*/

    void* image_base() const
    {
        auto const nt = nt_header();
        return unsafe_cast_from(nt->OptionalHeader.ImageBase);
    }

  public:
    /*ULONG size() const
    {
        return entry_full_->SizeOfImage;
    }*/

    wstring_view name() const
    {
        auto const& buff = entry_full_->BaseDllName;
        return {ubegin(buff), uend(buff)};
    }

    wstring_view path() const
    {
        auto const& buff = entry_full_->FullDllName;
        return {ubegin(buff), uend(buff)};
    }

    void* function(string_view name) const;
    void* vtable(string_view name) const;

  private:
    template <typename Fn>
    void* sections_find(Fn fn) const;

  public:
    template <class... Segment>
    auto find(pattern<Segment...> const& pat) const -> void*;
    auto find(section_string name) const -> IMAGE_SECTION_HEADER const*;
};

template <class... Segment>
void* library_info::find(pattern<Segment...> const& pat) const
{
    decltype(auto) front_pattern_segment = pat.front().view();
    auto const front_pattern_byte        = front_pattern_segment.front();

    if constexpr (sizeof...(Segment) == 1)
    {
        if (front_pattern_segment.size() == 1)
        {
            return sections_find([front_pattern_byte]<typename T>(T const section_first, T const section_last) -> T {
                return std::find(section_first, section_last, front_pattern_byte);
            });
        }
    }

    return sections_find([front_pattern_byte, &pat, pattern_length = pat.length()]<typename T>(T section_first, T const section_last) -> T {
        auto const section_last_safe = section_last - pattern_length;

        if (section_first == section_last_safe)
        {
            if (pat.equal(section_first))
                return section_first;
        }
        else if (section_first < section_last_safe)
            for (;;)
            {
                auto const section_front_pattern_byte = std::find(section_first, section_last_safe, front_pattern_byte);
                if (section_front_pattern_byte == section_last_safe)
                    break;
                if (pat.equal(section_front_pattern_byte))
                    return section_front_pattern_byte;

                section_first = section_front_pattern_byte + 1;
            }

        return section_last;
    });
}

inline namespace literals
{
#ifdef _DEBUG
inline library_info operator"" _dll(wchar_t const* name, size_t length)
{
    return {
        wstring_view{name, length},
        library_info::extension::dll
    };
}
#else
template <static_wstring Name>
library_info operator"" _dll()
{
    return {Name + library_info::extension::dll};
}
#endif

template <static_string Name>
library_info operator"" _dll()
{
    return {Name + library_info::extension::dll};
}

} // namespace literals

inline void* library_info::function(string_view const name) const
{
    auto const base_address = entry_full_->DllBaseAddress;

    auto const& data_dirs    = (nt_header()->OptionalHeader.DataDirectory);
    auto const& entry_export = data_dirs[IMAGE_DIRECTORY_ENTRY_EXPORT];
    auto const export_dir    = unsafe_cast<IMAGE_EXPORT_DIRECTORY*>(base_address + entry_export.VirtualAddress);
    // auto const export_dir_end = export_dir_start+entry_export.Size;

    auto const names = unsafe_cast<uint32_t*>(base_address + export_dir->AddressOfNames);
    auto const funcs = unsafe_cast<uint32_t*>(base_address + export_dir->AddressOfFunctions);
    auto const ords  = unsafe_cast<uint16_t*>(base_address + export_dir->AddressOfNameOrdinals);

    //----

    auto const name_length = name.length();
    auto const name_first  = ubegin(name);
    auto const name_last   = uend(name);

    auto const last_offset = std::min(export_dir->NumberOfNames, export_dir->NumberOfFunctions);
    for (DWORD offset = 0; offset != last_offset; ++offset)
    {
        auto const fn_name_raw = unsafe_cast<char const*>(base_address + names[offset]);
        if (fn_name_raw[name_length] != '\0')
            continue;
        if (!std::equal(name_first, name_last, fn_name_raw))
            continue;

        auto const fn = base_address + funcs[ords[offset]];
        // assert(fn > virtual_addr_start && fn < virtual_addr_end); // fwd export not implemented
        return unsafe_cast_from(fn);
    }

    return nullptr;
}

inline void* library_info::vtable(string_view name) const
{
    union
    {
        uintptr_t rtti_descriptor_address;
        void* rtti_descriptor;
        char const* rtti_descriptor_view;
    };

    // rtti_descriptor = find_rtti_descriptor(name, image_base, nt_header);
    if (auto const space = name.find(' '); space == name.npos)
    {
        rtti_descriptor = find(make_pattern(".?A", 1, name, 0, "@@"));
    }
    else
    {
        auto const object_name = name.substr(0, space);
        char object_tag;

        if (object_name == "struct")
            object_tag = 'U';
        else if (object_name == "class")
            object_tag = 'V';
        else
            unreachable();

        auto const class_name = name.substr(space + 1);

        rtti_descriptor = find(make_pattern(".?A", 0, object_tag, 0, class_name, 0, "@@"));
    }

    //---------

    // we're doing - 0x8 here, because the location of the rtti typedescriptor is 0x8 bytes before the std::string
    xref const type_descriptor{rtti_descriptor_address - sizeof(uintptr_t) * 2};

    // dos + section->VirtualAddress, section->SizeOfRawData

    auto const img_base = safe_cast<uint8_t>(image_base());

    auto const rdata       = find(section::rdata);
    auto const rdata_first = img_base + rdata->VirtualAddress;
    auto const rdata_last  = rdata_first + rdata->SizeOfRawData;

    auto const addr1 = std::find_if(rdata_first, rdata_last, [&type_descriptor](uint8_t const& found) -> bool {
        return std::equal(type_descriptor.begin(), type_descriptor.end(), &found) && *unsafe_cast<uint32_t*>(&found - 0x8) == 0;
    });
    if (addr1 == rdata_last)
        return nullptr;
    xref const unnamed_xref2{unsafe_cast<uintptr_t>(addr1) - 0xC};
    auto const addr2 = std::search(rdata_first, rdata_last, unnamed_xref2.begin(), unnamed_xref2.end());
    if (addr2 == rdata_last)
        return nullptr;

    auto const text       = find(section::text);
    auto const text_first = img_base + text->VirtualAddress;
    auto const text_last  = text_first + text->SizeOfRawData;
    xref const unnamed_xref3{unsafe_cast<uintptr_t>(addr2) + 4};
    auto const addr3 = std::search(text_first, text_last, unnamed_xref3.begin(), unnamed_xref3.end());
    if (addr3 == text_last)
        return nullptr;

    return (addr3);
}

template <typename Fn>
void* library_info::sections_find(Fn fn) const
{
    auto const nt       = nt_header();
    auto const img_base = safe_cast<uint8_t>(image_base());

    auto first_section      = IMAGE_FIRST_SECTION(nt);
    auto const last_section = first_section + nt->FileHeader.NumberOfSections;

    for (; first_section != last_section; ++first_section)
    {
        auto section_first      = img_base + first_section->VirtualAddress;
        auto const section_last = section_first + first_section->SizeOfRawData;

        auto const found = fn(section_first, section_last);
        if (found == section_last)
            continue;
        return found;
    }

    return nullptr;
}

inline IMAGE_SECTION_HEADER const* library_info::find(section_string const name) const
{
    auto const name_length = name.length();

    assert(name_length < sizeof(IMAGE_SECTION_HEADER::Name));

    auto const nt = nt_header();

    auto first_section      = IMAGE_FIRST_SECTION(nt);
    auto const last_section = first_section + nt->FileHeader.NumberOfSections;

    auto const name_first = ubegin(name);
    auto const name_last  = uend(name);

    for (; first_section != last_section; ++first_section)
    {
        if (first_section->Name[name_length] != '\0')
            continue;
        if (!std::equal(name_first, name_last, first_section->Name))
            continue;
        return first_section;
    }

    return nullptr;
}
} // namespace fd
