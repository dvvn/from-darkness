#include <fd/library_info/section.h>

#include <windows.h>
#include <winternl.h>

namespace fd
{

IMAGE_SECTION_HEADER *_find_section(IMAGE_NT_HEADERS *nt, const char *name, size_t length)
{
    auto begin = IMAGE_FIRST_SECTION(nt);
    auto end   = begin + static_cast<size_t>(nt->FileHeader.NumberOfSections);

    for (; begin != end; ++begin)
    {
        if (memcmp(begin->Name, name, length) == 0)
            return begin;
    }

    return nullptr;
}
}