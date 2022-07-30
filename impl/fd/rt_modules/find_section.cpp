module;

#include <fd/rt_modules/winapi_fwd.h>

module fd.rt_modules:find_section;
import :library_info;
import fd.string;

using namespace fd;

IMAGE_SECTION_HEADER* find_section(LDR_DATA_TABLE_ENTRY* const ldr_entry, const string_view name, const bool notify)
{
    if (!ldr_entry)
        return nullptr;

    IMAGE_SECTION_HEADER* found_header = nullptr;

    for (auto& header : dos_nt(ldr_entry).sections())
    {
        if (reinterpret_cast<const char*>(header.Name) == name)
        {
            found_header = &header;
            break;
        }
    }

    if (notify)
        library_info(ldr_entry).log("section", name, found_header);
    return found_header;
}
