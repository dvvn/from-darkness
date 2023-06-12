#pragma once

#include "core.h"

namespace fd
{
void *find_pattern(IMAGE_NT_HEADERS *nt, char const *pattern, size_t length);
void *find_pattern(IMAGE_NT_HEADERS *nt, struct string_view pattern);
} // namespace fd