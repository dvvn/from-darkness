#pragma once

#include "container/span.h"
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
struct library_section_view : span<uint8_t>
{
    library_section_view(IMAGE_SECTION_HEADER const* section, void* image_base);
};

struct library_sections_range : span<IMAGE_SECTION_HEADER>
{
    library_sections_range(IMAGE_NT_HEADERS* nt);
};

IMAGE_SECTION_HEADER* find(library_sections_range sections, char const* name, size_t name_length);

template <size_t L>
IMAGE_SECTION_HEADER* find(library_sections_range const sections, char const (&name)[L])
{
    return find(sections, name, L - 1);
}

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

    void* operator+(ULONG_PTR other) const
    {
        return reinterpret_cast<void*>(DllBaseAddress + other);
    }
};

struct library_data_dir_range : span<IMAGE_DATA_DIRECTORY>
{
    library_data_dir_range(IMAGE_NT_HEADERS* nt);
};

class basic_library_info
{
    union
    {
        LDR_DATA_TABLE_ENTRY_FULL* entry_full_;
        LDR_DATA_TABLE_ENTRY* entry_;
    };

  public:
    using sections_view  = library_section_view;
    using sections_range = library_sections_range;
    using data_dir_range = library_data_dir_range;

    struct extension_tag
    {
    };

    basic_library_info(char const* name, size_t length, wchar_t* buffer);
    basic_library_info(wchar_t const* name, size_t length);
    // basic_library_info(char const* name, size_t name_length, wchar_t* name_buffer, wchar_t const* extension, size_t extension_length);
    basic_library_info(wchar_t const* name, size_t name_length, wchar_t const* extension, size_t extension_length);

    basic_library_info(wchar_t const* name, size_t length, extension_tag);

    template <size_t Length>
    basic_library_info(wchar_t const (&name)[Length])
        : basic_library_info(name, Length - 1)
    {
    }

    template <size_t Length>
    basic_library_info(char const (&name)[Length])
    {
        wchar_t buffer[Length - 1];
        std::construct_at(this, name, Length - 1, buffer);
    }

#if 0
    template <size_t S>
    basic_library_info(library_tag<S> const& tag)
    {
        auto constexpr buffer_length = tag.static_length() + file_extension_length;

        char_type buffer[buffer_length];
        tag.add_extension(buffer, file_extension);

        entry_ = basic_library_info(buffer, buffer_length).entry_;
    }
#endif

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

    sections_range sections() const;
    data_dir_range data_dirs() const;
};

uint8_t* begin(basic_library_info const& info);
uint8_t* end(basic_library_info const& info);
} // namespace fd