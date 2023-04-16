#pragma once

#include <cstring>

using IMAGE_NT_HEADERS = struct _IMAGE_NT_HEADERS;

namespace fd
{
void *_find_pattern(IMAGE_NT_HEADERS *nt, char const *pattern, size_t length);

template <size_t S>
void *_find_pattern(IMAGE_NT_HEADERS *nt, const char (&name)[S])
{
    return _find_pattern(nt, name, S - 1);
}
} // namespace fd
