#include <fd/library_info/pattern.h>
#include <fd/mem_scanner.h>

#include <windows.h>
#include <winternl.h>

namespace fd
{
void *find_pattern(IMAGE_NT_HEADERS *nt, char const *pattern, size_t length)
{
    auto scanner = pattern_scanner(nt->OptionalHeader.ImageBase, nt->OptionalHeader.SizeOfImage);
    return scanner(pattern, length).front();
}
}