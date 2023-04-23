#pragma once

#include <cstddef>

using LDR_DATA_TABLE_ENTRY = struct _LDR_DATA_TABLE_ENTRY;
using IMAGE_SECTION_HEADER = struct _IMAGE_SECTION_HEADER;
using IMAGE_DOS_HEADERS    = struct _IMAGE_DOS_HEADERS;
using IMAGE_NT_HEADERS     = struct _IMAGE_NT_HEADERS;

namespace fd
{
void *find_rtti_descriptor(IMAGE_NT_HEADERS *nt, char const *name, size_t length);
void *find_vtable(
    IMAGE_SECTION_HEADER *rdata,
    IMAGE_SECTION_HEADER *text,
    IMAGE_DOS_HEADER *dos,
    void const *rtti_decriptor);
}