#pragma once

#include "tier1/container/span.h"
#include "tier1/functional/cast.h"
#include "tier1/memory/xref.h"
#include "tier1/pattern.h"
#include "tier1/pattern/make.h"
#include "tier1/string/static.h"
#include "tier1/string/view.h"
#include "tier2/core.h"

#include <windows.h>
#include <winternl.h>

#include <algorithm>
#include <cassert>

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

inline size_t size(UNICODE_STRING const& ustr)
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

namespace FD_TIER2(native)
{
class interface_register;
}

namespace FD_TIER(2)
{
union library_base_address
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
        using string_view::basic_string_view;
    };

    struct section
    {
        static constexpr section_string rdata = ".rdata";
        static constexpr section_string text  = ".text";
    };

    template <class T>
    library_info(T const& name)
        // todo: add string_begin/end func
        requires(std::same_as<typename T::value_type, wchar_t>)
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

    template <class N, class E>
    library_info(N const& name, E const& ext) requires(std::same_as<typename N::value_type, wchar_t> && std::same_as<typename E::value_type, wchar_t>)
    {
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
            if (size(entry->BaseDllName) != full_name_length)
                continue;
            if (!std::equal(ext_first, ext_last, begin(entry->BaseDllName) + name_length))
                continue;
            if (!std::equal(name_first, name_last, begin(entry->BaseDllName)))
                continue;

            entry_full_ = entry;
            return;
        }

        entry_full_ = nullptr;
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

    library_base_address base() const
    {
        return {entry_full_->DllBase};
    }

    void* image_base() const
    {
        auto const nt = nt_header();
        return unsafe_cast<void*>(nt->OptionalHeader.ImageBase);
    }

  public:
    /*ULONG size() const
    {
        return entry_full_->SizeOfImage;
    }*/

    wstring_view name() const
    {
        auto const& buff = entry_full_->BaseDllName;
        return {begin(buff), end(buff)};
    }

    wstring_view path() const
    {
        auto const& buff = entry_full_->FullDllName;
        return {begin(buff), end(buff)};
    }

    void* function(string_view name) const;
    void* vtable(string_view name) const;

  private:
    template <typename Fn>
    void* sections_find(Fn fn) const
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

struct native_library_info : library_info
{
    using interface_register = native::interface_register;

    using library_info::library_info;

    interface_register* root_interface() const;
};

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

inline native_library_info operator"" _dlln(wchar_t const* name, size_t length)
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
    return {to_string(Name, library_info::extension::dll)};
}

template <static_wstring Name>
native_library_info operator"" _dlln()
{
    return {to_string(Name, library_info::extension::dll)};
}
#endif

template <static_string Name>
library_info operator"" _dll()
{
    return {to_string(Name, library_info::extension::dll)};
}

template <static_string Name>
native_library_info operator"" _dlln()
{
    return {to_string(Name, library_info::extension::dll)};
}

} // namespace literals

inline void* library_info::function(string_view name) const
{
    auto const base_address = (this->base());

    auto const& data_dirs    = (this->nt_header()->OptionalHeader.DataDirectory);
    auto const& entry_export = data_dirs[IMAGE_DIRECTORY_ENTRY_EXPORT];
    auto const export_dir    = safe_cast<IMAGE_EXPORT_DIRECTORY>(base_address + entry_export.VirtualAddress);
    // auto const export_dir_end = export_dir_start+entry_export.Size;

    auto const names = safe_cast<uint32_t>(base_address + export_dir->AddressOfNames);
    auto const funcs = safe_cast<uint32_t>(base_address + export_dir->AddressOfFunctions);
    auto const ords  = safe_cast<uint16_t>(base_address + export_dir->AddressOfNameOrdinals);

    //----

    auto const name_length = name.length();
    auto const name_first  = ubegin(name);
    auto const name_last   = uend(name);

    auto const last_offset = std::min(export_dir->NumberOfNames, export_dir->NumberOfFunctions);
    for (DWORD offset = 0; offset != last_offset; ++offset)
    {
        auto const fn_name_raw = safe_cast<char>(base_address + names[offset]);
        if (fn_name_raw[name_length] != '\0')
            continue;
        if (!std::equal(name_first, name_last, fn_name_raw))
            continue;

        void* fn = base_address + funcs[ords[offset]];
        // assert(fn > virtual_addr_start && fn < virtual_addr_end); // fwd export not implemented
        return fn;
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
    auto const rdata_begin = img_base + rdata->VirtualAddress;
    auto const rdata_end   = rdata_begin + rdata->SizeOfRawData;

    auto const addr1 = std::find_if(rdata_begin, rdata_end, [&type_descriptor](uint8_t const& found) -> bool {
        return std::equal(type_descriptor.begin(), type_descriptor.end(), &found) && *unsafe_cast<uint32_t*>(&found - 0x8) == 0;
    });
    if (addr1 == rdata_end)
        return nullptr;
    xref const unnamed_xref2{unsafe_cast<uintptr_t>(addr1) - 0xC};
    auto const addr2 = std::search(rdata_begin, rdata_end, unnamed_xref2.begin(), unnamed_xref2.end());
    if (addr2 == rdata_end)
        return nullptr;

    auto const text       = find(section::text);
    auto const text_begin = img_base + text->VirtualAddress;
    auto const text_end   = text_begin + text->SizeOfRawData;
    xref const unnamed_xref3{unsafe_cast<uintptr_t>(addr2) + 4};
    auto const addr3 = std::search(text_begin, text_end, unnamed_xref3.begin(), unnamed_xref3.end());
    if (addr3 == text_end)
        return nullptr;

    return (addr3);
}

inline IMAGE_SECTION_HEADER const* library_info::find(section_string const name) const
{
    auto const name_length = name.length();

    assert(name_length < sizeof(IMAGE_SECTION_HEADER::Name));

    auto nt = nt_header();

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

// constexpr wstring_view to_string(library_info::extension::value_type const value)
//{
//     using enum library_info::extension::value_type;
//     switch (value)
//     {
//     case dll:
//         return L".dll";
//     case exe:
//         return L".exe";
//     default:
//         return {nullptr, 1};
//     }
// }
} // namespace FD_TIER(2)
