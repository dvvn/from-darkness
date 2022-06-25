module;

#include <fd/core/callback.h>

#include <windows.h>
#include <winternl.h>

#include <string_view>

export module fd.rt_modules:find_section;

FD_CALLBACK(on_section_found, const LDR_DATA_TABLE_ENTRY* /* library name */, std::string_view /*section name*/, IMAGE_SECTION_HEADER* /*section ptr*/);

IMAGE_SECTION_HEADER* find_section(LDR_DATA_TABLE_ENTRY* const ldr_entry, const std::string_view name, const bool notify = true);

export namespace fd
{
    using ::find_section;
    using ::on_section_found;
} // namespace fd