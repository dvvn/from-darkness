#pragma once

#include "core.h"

namespace fd
{
LDR_DATA_TABLE_ENTRY *find_library(void *base_address);
LDR_DATA_TABLE_ENTRY *find_library(wchar_t const *name, size_t length);

template <size_t S>
LDR_DATA_TABLE_ENTRY *find_library(wchar_t const (&name)[S])
{
    return find_library(name, S - 1);
}

extern LDR_DATA_TABLE_ENTRY *const this_library;
}