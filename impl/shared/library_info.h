#pragma once

#include "functional/cast.h"
#include "memory/xref.h"
#include "pattern/fwd.h"
#include "string/view.h"

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

  protected:
    class basic_object_getter
    {
      protected:
        library_info const* linfo_;

      public:
        basic_object_getter(library_info const* linfo)
            : linfo_{linfo}
        {
        }
    };

    struct basic_section_getter : basic_object_getter
    {
        using pointer = IMAGE_SECTION_HEADER const*;

      protected:
        pointer find(string_view name) const;

      public:
        pointer rdata() const
        {
            return find(".rdata");
        }

        pointer text() const
        {
            return find(".text");
        }
    };

    class basic_pattern_getter : protected basic_section_getter
    {
        template <typename Fn>
        void* find_in_section(Fn fn) const;
        template <typename Fn>
        void* find_anywhere(Fn fn) const;

      protected:
        template <class... Segment>
        auto find(pattern<Segment...> const& pat) const -> void*;
    };

    struct basic_function_getter : basic_object_getter
    {
      protected:
        void* find(string_view name) const;
    };

  public:
    library_info(wstring_view name);
    library_info(wstring_view name, wstring_view ext);

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

    /*uint8_t* begin() const
    {
        return safe_cast_from(entry_full_->DllBase);
    }

    uint8_t* end() const
    {
        return unsafe_cast_from(entry_full_->DllBaseAddress + entry_full_->SizeOfImage);
    }*/

    uint8_t* data() const
    {
        return safe_cast_from(entry_full_->DllBase);
    }

    ULONG size() const
    {
        return entry_full_->SizeOfImage;
    }

  public:
    wstring_view name() const
    {
        using std::data;
        using std::size;
        auto const& buff = entry_full_->BaseDllName;
        return {data(buff), size(buff)};
    }

    wstring_view path() const
    {
        using std::data;
        using std::size;
        auto const& buff = entry_full_->FullDllName;
        return {data(buff), size(buff)};
    }

    // void* vtable(string_view name) const;
};

inline library_info::library_info(wstring_view const name)
{
    auto const name_first = name.data();
    auto const name_last  = name_first + name.length();

    auto const root_list = module_list();
    for (auto list_entry = root_list->Flink; list_entry != root_list; list_entry = list_entry->Flink)
    {
        auto const entry = ldr_table(list_entry);
        if (!entry->BaseDllName)
            continue;
        using pointer = wstring_view::const_pointer;
        if (!std::equal<pointer, pointer>(name_first, name_last, begin(entry->BaseDllName), end(entry->BaseDllName)))
            continue;

        entry_full_ = entry;
        return;
    }

    entry_full_ = nullptr;
}

inline library_info::library_info(wstring_view const name, wstring_view const ext)
{
    using std::data;
    using std::size;

    auto const name_length = size(name);
    auto const name_first  = data(name);
    auto const name_last   = name_first + name_length;

    auto const ext_length = size(ext);
    auto const ext_first  = data(ext);
    auto const ext_last   = ext_first + ext_length;

    auto const full_name_length = name_length + ext_length;

    auto const root_list = module_list();
    for (auto list_entry = root_list->Flink; list_entry != root_list; list_entry = list_entry->Flink)
    {
        using std::equal;

        auto const entry = ldr_table(list_entry);
        if (!entry->BaseDllName)
            continue;
        if (full_name_length != size(entry->BaseDllName))
            continue;

        auto const entry_first = data(entry->BaseDllName);
        if (!equal(ext_first, ext_last, entry_first + name_length))
            continue;
        if (!equal(name_first, name_last, entry_first))
            continue;

        entry_full_ = entry;
        return;
    }

    entry_full_ = nullptr;
}

#if 0
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
#endif

} // namespace fd
