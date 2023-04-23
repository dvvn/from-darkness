#pragma once

#include <cstddef>

using IMAGE_NT_HEADERS = struct _IMAGE_NT_HEADERS;

namespace fd
{
void *find_pattern(IMAGE_NT_HEADERS *nt, char const *pattern, size_t length);

template <size_t S>
void *find_pattern(IMAGE_NT_HEADERS *nt, char const (&name)[S])
{
    return find_pattern(nt, name, S - 1);
}
} // namespace fd
