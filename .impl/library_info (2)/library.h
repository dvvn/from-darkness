#pragma once

#include "core.h"

namespace fd
{
system_library_entry find_library(void *base_address);
system_library_entry find_library(system_string_view name);
}