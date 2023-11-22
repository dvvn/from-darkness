#pragma once

#include "container/span.h"
#include "string/static.h"
#include "string/view.h"

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

namespace fd
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
        } value;

        consteval extension_tag(value_type const val)
            : value{val}
        {
        }
    };

    static constexpr auto extension_tag_dll = extension_tag::dll;
    static constexpr auto extension_tag_exe = extension_tag::exe;

    basic_library_info(wchar_t const* name, size_t length);
    basic_library_info(wchar_t const* name, size_t length, extension_tag ext);

    template <size_t Length>
    basic_library_info(wchar_t const (&name)[Length])
        : basic_library_info(name, Length - 1)
    {
    }

  private:
    template <size_t Length>
    basic_library_info(static_wstring<Length> const& name)
        : basic_library_info(name.data(), name.length())
    {
    }

    template <size_t Length>
    basic_library_info(static_wstring<Length> const& name, extension_tag const ext)
        : basic_library_info(name.data(), name.length(), ext)
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
};

IMAGE_SECTION_HEADER* find(basic_library_info::sections_range sections, char const* name, size_t name_length);

template <size_t L>
IMAGE_SECTION_HEADER* find(basic_library_info::sections_range const sections, char const (&name)[L])
{
    return find(sections, name, L - 1);
}

uint8_t* begin(basic_library_info const& info);
uint8_t* end(basic_library_info const& info);
} // namespace fd