#pragma once

#include "string/view.h"

#include <windows.h>
#include <winternl.h>

#include <cstddef>
#include <cstdint>

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

struct basic_library_info
{
    using string_type = wstring_view;
    using char_type   = wchar_t;

    static constexpr auto file_extension        = ".dll";
    static constexpr auto file_extension_length = 4;

  private:
    union
    {
        LDR_DATA_TABLE_ENTRY_FULL* entry_full_;
        LDR_DATA_TABLE_ENTRY* entry_;
    };

  public:
    basic_library_info(char_type const* name, size_t length);

    template <size_t Length>
    basic_library_info(char_type const (&name)[Length])
        : basic_library_info(name, Length - 1)
    {
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

    void* base() const;
    void* image_base() const;
    size_t length() const;
    string_type name() const;
    string_type path() const;
    IMAGE_DATA_DIRECTORY* directory(uint8_t index) const;
    IMAGE_SECTION_HEADER* section(char const* name, uint8_t length) const;

    template <size_t Length>
    IMAGE_SECTION_HEADER* section(char const (&name)[Length]) const
    {
        return section(name, Length - 1);
    }
};
} // namespace fd