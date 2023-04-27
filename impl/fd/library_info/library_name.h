#pragma once

#include "core.h"

#include <string_view>

namespace fd
{
std::wstring_view library_path(LDR_DATA_TABLE_ENTRY *entry);
[[deprecated]]
std::wstring_view library_name(LDR_DATA_TABLE_ENTRY *entry);
std::wstring_view library_name(std::wstring_view path);
}