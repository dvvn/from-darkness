#include "pattern.h"

#include <fd/mem_scanner.h>

#include <windows.h>
#include <winternl.h>

namespace fd
{
void *find_pattern(IMAGE_NT_HEADERS *nt, char const *pattern, size_t length)
{
    auto begin = reinterpret_cast<uint8_t *>(nt->OptionalHeader.ImageBase);
    auto end   = begin + nt->OptionalHeader.SizeOfImage;
    return find_pattern(begin, end, pattern, length);
}
}