#pragma once

#include <cstddef>
#include <cstdint>

// ReSharper disable CppInconsistentNaming

using LDR_DATA_TABLE_ENTRY      = struct _LDR_DATA_TABLE_ENTRY;
using LDR_DATA_TABLE_ENTRY_FULL = struct _LDR_DATA_TABLE_ENTRY_FULL;
using IMAGE_DOS_HEADER          = struct _IMAGE_DOS_HEADER;
using IMAGE_SECTION_HEADER      = struct _IMAGE_SECTION_HEADER;
using IMAGE_EXPORT_DIRECTORY    = struct _IMAGE_EXPORT_DIRECTORY;
using IMAGE_DATA_DIRECTORY      = struct _IMAGE_DATA_DIRECTORY;

#ifdef _WIN64
using IMAGE_NT_HEADERS = struct _IMAGE_NT_HEADERS64;
#else
using IMAGE_NT_HEADERS = struct _IMAGE_NT_HEADERS;
#endif

// ReSharper restore CppInconsistentNaming

namespace fd
{
struct wstring_view;

template <size_t S>
struct library_tag;

struct basic_library_info
{
    using string_type = wstring_view;
    using char_type   = wchar_t;

    static constexpr auto file_extension        = ".dll";
    static constexpr auto file_extension_length = 4;

  private:
    union
    {
        LDR_DATA_TABLE_ENTRY_FULL *entry_full_;
        LDR_DATA_TABLE_ENTRY *entry_;
    };

  public:
    basic_library_info(char_type const *name, size_t length);

    template <size_t Length>
    basic_library_info(char_type (&name)[Length])
        : basic_library_info(name, Length - 1)
    {
    }

    template <size_t S>
    basic_library_info(library_tag<S> const &tag)
    {
        constexpr auto buffer_length = tag.static_length() + file_extension_length;

        char_type buffer[buffer_length];
        tag.add_extension(buffer, file_extension);

        entry_ = basic_library_info(buffer, buffer_length).entry_;
    }

    void *base() const;
    void *image_base() const;
    size_t length() const;
    string_type name() const;
    string_type path() const;
    IMAGE_DATA_DIRECTORY *directory(uint8_t index) const;
    IMAGE_SECTION_HEADER *section(char const *name, uint8_t length) const;

    template <size_t Length>
    IMAGE_SECTION_HEADER *section(char const (&name)[Length]) const
    {
        return section(name, Length - 1);
    }
};
} // namespace fd