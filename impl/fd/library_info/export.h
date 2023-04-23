#pragma once

#include <cstddef>

using LDR_DATA_TABLE_ENTRY = struct _LDR_DATA_TABLE_ENTRY;
using IMAGE_NT_HEADERS     = struct _IMAGE_NT_HEADERS;
using IMAGE_DOS_HEADER     = struct _IMAGE_DOS_HEADER;

namespace fd
{
void *find_export(IMAGE_DOS_HEADER *dos, IMAGE_NT_HEADERS *nt, char const *name, size_t length);

template <size_t S>
void *find_export(IMAGE_DOS_HEADER *dos, IMAGE_NT_HEADERS *nt, char const (&name)[S])
{
    return find_export(dos, nt, name, S - 1);
}
}