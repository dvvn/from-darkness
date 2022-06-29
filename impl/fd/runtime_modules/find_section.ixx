module;

#include <windows.h>
#include <winternl.h>

#include <string_view>

export module fd.rt_modules:find_section;

IMAGE_SECTION_HEADER* find_section(LDR_DATA_TABLE_ENTRY* const ldr_entry, const std::string_view name, const bool notify = true);

export namespace fd
{
    using ::find_section;
} // namespace fd
