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
class basic_library_info
{
    union
    {
        LDR_DATA_TABLE_ENTRY_FULL* entry_full_;
        LDR_DATA_TABLE_ENTRY* entry_;
    };

  public:
    basic_library_info(char const* name, size_t length, wchar_t* buffer);
    basic_library_info(wchar_t const* name, size_t length);
    // basic_library_info(char const* name, size_t name_length, wchar_t* name_buffer, wchar_t const* extension, size_t extension_length);
    basic_library_info(wchar_t const* name, size_t name_length, wchar_t const* extension, size_t extension_length);

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

    void* base() const;
    void* image_base() const;
    size_t length() const;
    wstring_view name() const;
    wstring_view path() const;
    IMAGE_DATA_DIRECTORY* directory(size_t index) const;
    IMAGE_SECTION_HEADER* section(char const* name, size_t length) const;

    template <size_t Length>
    IMAGE_SECTION_HEADER* section(char const (&name)[Length]) const
    {
        return section(name, Length - 1);
    }
};

uint8_t* begin(basic_library_info const& info);
uint8_t* end(basic_library_info const& info);

span<uint8_t> make_library_section_view(IMAGE_SECTION_HEADER const* section, void* image_base);

struct library_section_view : span<uint8_t>
{
    library_section_view(IMAGE_SECTION_HEADER const* section, void* image_base);
};
} // namespace fd