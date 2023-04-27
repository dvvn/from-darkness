#pragma once

#include "core.h"

namespace fd
{
void *find_rtti_descriptor(IMAGE_NT_HEADERS *nt, char const *name, size_t length);

template <size_t S>
void *find_rtti_descriptor(IMAGE_NT_HEADERS *nt, char const (&name)[S])
{
    return find_rtti_descriptor(nt, name, S - 1);
}

void *find_vtable(IMAGE_SECTION_HEADER *rdata, IMAGE_SECTION_HEADER *text, IMAGE_DOS_HEADER *dos, void *rtti_decriptor);
}