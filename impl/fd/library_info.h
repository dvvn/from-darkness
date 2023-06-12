#pragma once

#include <cstdint>

// ReSharper disable CppInconsistentNaming

using LDR_DATA_TABLE_ENTRY = struct _LDR_DATA_TABLE_ENTRY;
using IMAGE_DOS_HEADER     = struct _IMAGE_DOS_HEADER;
using IMAGE_SECTION_HEADER = struct _IMAGE_SECTION_HEADER;

#ifdef _WIN64
using IMAGE_NT_HEADERS = struct _IMAGE_NT_HEADERS64;
#else
using IMAGE_NT_HEADERS = struct _IMAGE_NT_HEADERS;
#endif

namespace fd
{
using system_library_entry  = LDR_DATA_TABLE_ENTRY const *;
using system_section_header = IMAGE_SECTION_HEADER const *;

using system_cstring     = wchar_t const *;
using system_string_view = struct wstring_view;

struct string_view;

template <typename T>
struct span;

class system_library
{
    system_library_entry entry_;

  public:
    system_library(system_string_view name);

    system_string_view name() const;
    system_string_view path() const;
    span<uint8_t> memory() const;
    void *function(string_view name) const;
    void *pattern(string_view pattern) const;
    system_section_header section(string_view name) const;
    void *rtti_descriptor(string_view class_name) const;
    void *vtable(string_view name) const;
};
} // namespace fd