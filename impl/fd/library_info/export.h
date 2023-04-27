#pragma once

#include "core.h"

namespace fd
{
void *find_export(IMAGE_DOS_HEADER *dos, IMAGE_NT_HEADERS *nt, char const *name, size_t length);

template <size_t S>
void *find_export(IMAGE_DOS_HEADER *dos, IMAGE_NT_HEADERS *nt, char const (&name)[S])
{
    return find_export(dos, nt, name, S - 1);
}
}