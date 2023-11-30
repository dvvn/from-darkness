#pragma once

#include "tier1/container/span.h"
#include "tier1/string/static.h"
#include "tier1/string/view.h"

// ReSharper disable once CppUnusedIncludeDirective
#include <windows.h>
#include <winternl.h>

// ReSharper disable CppInconsistentNaming

// see https://www.vergiliusproject.com/kernels/x64/Windows%2011/22H2%20(2022%20Update)/_LDR_DATA_TABLE_ENTRY
struct LDR_DATA_TABLE_ENTRY_FULL
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
};

// ReSharper restore CppInconsistentNaming

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

class basic_library_info
{
    union
    {
        LDR_DATA_TABLE_ENTRY_FULL* entry_full_;
        LDR_DATA_TABLE_ENTRY* entry_;
    };

  public:
    struct data_dir_range : span<IMAGE_DATA_DIRECTORY>
    {
        data_dir_range(IMAGE_NT_HEADERS* nt);
    };

    struct sections_range : span<IMAGE_SECTION_HEADER>
    {
        sections_range(IMAGE_NT_HEADERS* nt);
    };

    struct section_view : span<uint8_t>
    {
        section_view(IMAGE_SECTION_HEADER const* section, void* image_base);
    };

    struct extension_tag
    {
        enum value_type : uint8_t
        {
            dll,
            exe
        };

        friend constexpr wstring_view to_string(value_type value);

      private:
#ifdef _DEBUG
        value_type value_;
#else
        wstring_view ext_;
#endif

      public:
        consteval extension_tag(value_type const val)
            :
#ifdef _DEBUG
            value_(val)
#else
            ext_(to_string(val));
#endif
        {
        }

        wstring_view get() const;
    };

    basic_library_info(wstring_view name);
    basic_library_info(wstring_view name, extension_tag ext);

  private:
    template <size_t Length>
    basic_library_info(static_wstring<Length> const& name)
        : basic_library_info(wstring_view(name))
    {
    }

    template <size_t Length>
    basic_library_info(static_wstring<Length> const& name, extension_tag const ext)
        : basic_library_info(wstring_view{name}, ext)
    {
    }

  public:
    template <size_t Length>
    basic_library_info(static_string<Length> const& name)
        : basic_library_info(static_wstring<Length>{name.begin(), name.end()})
    {
    }

    template <size_t Length>
    basic_library_info(static_string<Length> const& name, extension_tag const ext)
        : basic_library_info(static_wstring<Length>{name.begin(), name.end()}, ext)
    {
    }

    explicit operator bool() const
    {
        return entry_ != nullptr;
    }

    IMAGE_DOS_HEADER* dos_header() const;
    IMAGE_NT_HEADERS* nt_header() const;
    IMAGE_NT_HEADERS* nt_header(IMAGE_DOS_HEADER const* dos) const;

    library_base_address base() const;
    static void* image_base(IMAGE_NT_HEADERS* nt);
    void* image_base() const;
    size_t length() const;
    wstring_view name() const;
    wstring_view path() const;

    [[deprecated]]
    sections_range sections() const;
    [[deprecated]]
    data_dir_range data_dirs() const;
}; // namespace FD_TIER(2)

template <class LibraryInfo = basic_library_info>
auto unwrap_section(IMAGE_SECTION_HEADER const* section, void* image_base)
{
    typename LibraryInfo::section_view const section_view{section, image_base};
    return std::pair{ubegin(section_view), uend(section_view)};
}

template <class LibraryInfo = basic_library_info>
auto unwrap_section(typename LibraryInfo::sections_range::iterator section, void* image_base)
{
    typename LibraryInfo::section_view const section_view{unwrap_iterator(section), image_base};
    return std::pair{begin(section_view), end(section_view)};
}

constexpr wstring_view to_string(basic_library_info::extension_tag::value_type const value)
{
    using enum basic_library_info::extension_tag::value_type;
    switch (value)
    {
    case dll:
        return L".dll";
    case exe:
        return L".exe";
    default:
        return {nullptr, 1};
    }
}

uint8_t* begin(basic_library_info const& info);
uint8_t* end(basic_library_info const& info);
} // namespace FD_TIER(2)