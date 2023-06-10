#pragma once

#include "core.h"

namespace fd
{
struct library_cache
{
    virtual ~library_cache() = default;

    virtual void store(LDR_DATA_TABLE_ENTRY *entry)                              = 0;
    virtual LDR_DATA_TABLE_ENTRY *find(void *base_address) const                 = 0;
    virtual LDR_DATA_TABLE_ENTRY *find(system_cstring name, size_t length) const = 0;
    virtual LDR_DATA_TABLE_ENTRY *find(system_string_view name) const            = 0;
};

LDR_DATA_TABLE_ENTRY *find_library(void *base_address,library_cache*cache=0);
LDR_DATA_TABLE_ENTRY *find_library(system_cstring name, size_t length,library_cache*cache=0);
LDR_DATA_TABLE_ENTRY *find_library(system_string_view name,library_cache*cache=0);

extern LDR_DATA_TABLE_ENTRY *const this_library;
}