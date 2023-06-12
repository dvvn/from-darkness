#pragma once

#include "core.h"

namespace fd
{
system_string_view library_path(system_library_entry entry);
system_string_view library_name(system_library_entry entry);
system_string_view library_name(system_library_entry entry, size_t limit);
bool library_has_name(system_library_entry entry,system_string_view name);
}