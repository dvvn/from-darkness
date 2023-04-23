#pragma once

#include <cstddef>

using LDR_DATA_TABLE_ENTRY = struct _LDR_DATA_TABLE_ENTRY;
using IMAGE_NT_HEADERS     = struct _IMAGE_NT_HEADERS;
using IMAGE_SECTION_HEADER = struct _IMAGE_SECTION_HEADER;

namespace fd
{
IMAGE_SECTION_HEADER *find_section(IMAGE_NT_HEADERS *nt, const char *name, size_t length);

template <size_t S>
IMAGE_SECTION_HEADER *find_section(IMAGE_NT_HEADERS *nt, const char (&name)[S])
{
    return find_section(nt, name, S - 1);
}
}