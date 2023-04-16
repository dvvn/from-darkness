#pragma once

#include <cstdint>

using LDR_DATA_TABLE_ENTRY = struct _LDR_DATA_TABLE_ENTRY;
using IMAGE_NT_HEADERS     = struct _IMAGE_NT_HEADERS;
using IMAGE_DOS_HEADER     = struct _IMAGE_DOS_HEADER;

namespace fd
{
void *_find_export(IMAGE_DOS_HEADER *dos, IMAGE_NT_HEADERS *nt, const char *name, size_t length);

template <size_t S>
void *_find_export(IMAGE_DOS_HEADER *dos, IMAGE_NT_HEADERS *nt, const char (&name)[S])
{
    return _find_export(dos, nt, name, S - 1);
}
}