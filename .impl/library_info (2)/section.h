#pragma once

#include "core.h"

namespace fd
{
IMAGE_SECTION_HEADER *find_section(IMAGE_NT_HEADERS *nt, char const *name, size_t length);
}