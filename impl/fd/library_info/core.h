#pragma once

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
using system_cstring      = wchar_t const *;
using system_string_view = struct wstring_view;
}