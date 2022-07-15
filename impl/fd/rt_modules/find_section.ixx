module;

#include <windows.h>
#include <winternl.h>

export module fd.rt_modules:find_section;
export import fd.string;

IMAGE_SECTION_HEADER* find_section(LDR_DATA_TABLE_ENTRY* const ldr_entry, const fd::string_view name, const bool notify = true);

export namespace fd
{
    using ::find_section;
} // namespace fd
