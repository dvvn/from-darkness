#include "header.h"
#include "pattern.h"

#include <fd/mem_scanner.h>
#include <fd/tool/span.h>
#include <fd/tool/string_view.h>

namespace fd
{
void *find_pattern(IMAGE_NT_HEADERS *nt, char const *pattern, size_t length)
{
    auto [begin, size] = get_memory_range(nt);
    return find_pattern(begin, begin + size, pattern, length);
}

void *find_pattern(IMAGE_NT_HEADERS *nt, string_view pattern)
{
    auto [begin, size] = get_memory_range(nt);
    return find_pattern(begin, begin + size, pattern.data(), pattern.length());
}
}