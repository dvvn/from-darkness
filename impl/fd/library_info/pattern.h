#pragma once

#include "core.h"

namespace fd
{
void *find_pattern(IMAGE_NT_HEADERS *nt, char const *pattern, size_t length);

template <size_t S>
void *find_pattern(IMAGE_NT_HEADERS *nt, char const (&pattern)[S])
{
    return find_pattern(nt, pattern, S - 1);
}
} // namespace fd