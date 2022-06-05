module;

#include <fds/runtime_modules/notification.h>

#include <windows.h>
#include <winternl.h>

#include <string_view>

module fds.rt_modules:find_section;
import :helpers;

FDS_RTM_NOTIFICATION_IMPL(on_section_found);

IMAGE_SECTION_HEADER* find_section(LDR_DATA_TABLE_ENTRY* const ldr_entry, const std::string_view name, const bool notify)
{
    const auto [dos, nt] = fds::dos_nt(ldr_entry);

    const auto number_of_sections  = nt->FileHeader.NumberOfSections;
    const auto first_header        = IMAGE_FIRST_SECTION(nt);
    const auto last_section_header = first_header + number_of_sections;

    IMAGE_SECTION_HEADER* found_header = nullptr;

    for (auto header = first_header; header != last_section_header; ++header)
    {
        if (std::memcmp(header->Name, name.data(), name.size()) != 0)
            continue;
        if (header->Name[name.size()] != '\0')
            continue;
        found_header = header;
        break;
    }

    if (notify)
        std::invoke(on_section_found, ldr_entry, name, found_header);

    return found_header;
}
