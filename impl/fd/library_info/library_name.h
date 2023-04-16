#pragma once

#include <string_view>

using LDR_DATA_TABLE_ENTRY = struct _LDR_DATA_TABLE_ENTRY;

namespace fd
{
std::wstring_view library_path(LDR_DATA_TABLE_ENTRY *entry);
[[deprecated]]
std::wstring_view library_name(LDR_DATA_TABLE_ENTRY *entry);
std::wstring_view library_name(std::wstring_view path);
}