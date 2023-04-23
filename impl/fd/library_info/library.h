﻿#pragma once

#include <cstddef>

using LDR_DATA_TABLE_ENTRY = struct _LDR_DATA_TABLE_ENTRY;

namespace fd
{
LDR_DATA_TABLE_ENTRY *find_library(void *base_address);
LDR_DATA_TABLE_ENTRY *find_library(wchar_t const *name, size_t length);
extern LDR_DATA_TABLE_ENTRY *const this_library;
}