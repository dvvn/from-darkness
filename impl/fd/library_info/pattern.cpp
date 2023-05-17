#include "pattern.h"

#include <fd/magic_cast.h>
#include <fd/mem_scanner.h>

#include <windows.h>
#include <winternl.h>

namespace fd
{
void *find_pattern(IMAGE_NT_HEADERS *nt, char const *pattern, size_t length)
{
    to<uint8_t *> begin = (nt->OptionalHeader.ImageBase);
    auto end            = begin + nt->OptionalHeader.SizeOfImage;
    return find_pattern(begin, end, pattern, length);
}
}