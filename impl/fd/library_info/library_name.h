#pragma once

#include "core.h"

#include <fd/tool/string_view.h>

namespace fd
{
system_string_view library_path(LDR_DATA_TABLE_ENTRY *entry);
system_string_view library_name(LDR_DATA_TABLE_ENTRY *entry);
bool valid_library_name(LDR_DATA_TABLE_ENTRY *entry, system_string_view name);
// system_string_view library_name(system_string_view path);
// bool valid_library_name(system_string_view path,system_string_view name);
}