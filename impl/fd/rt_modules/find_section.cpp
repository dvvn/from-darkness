module;

#include <windows.h>
#include <winternl.h>

module fd.rt_modules:find_section;
import :helpers;
import :library_info;
import fd.string;

using namespace fd;

IMAGE_SECTION_HEADER* find_section(LDR_DATA_TABLE_ENTRY* const ldr_entry, const string_view name, const bool notify)
{
    if (!ldr_entry)
        return nullptr;

    const auto [dos, nt] = dos_nt(ldr_entry);

    const auto number_of_sections  = nt->FileHeader.NumberOfSections;
    const auto first_header        = IMAGE_FIRST_SECTION(nt);
    const auto last_section_header = first_header + number_of_sections;

    IMAGE_SECTION_HEADER* found_header = nullptr;

    for (auto header = first_header; header != last_section_header; ++header)
    {
        if (reinterpret_cast<const char*>(header->Name) == name)
        {
            found_header = header;
            break;
        }
    }

    if (notify)
        library_info(ldr_entry).log("section", name, found_header);
    return found_header;
}
