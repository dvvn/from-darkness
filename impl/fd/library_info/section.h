#pragma once

#include "core.h"

namespace fd
{
IMAGE_SECTION_HEADER *find_section(IMAGE_NT_HEADERS *nt, char const *name, size_t length);

template <size_t S>
IMAGE_SECTION_HEADER *find_section(IMAGE_NT_HEADERS *nt, char const (&name)[S])
{
    return find_section(nt, name, S - 1);
}
}