module;

#include <fd/core/callback.h>

#include <windows.h>
#include <winternl.h>

#include <string_view>

export module fd.rt_modules:find_export;

FD_CALLBACK(on_export_found, const LDR_DATA_TABLE_ENTRY* /* library name */, std::string_view /*export name*/, void* /*export ptr*/);

void* find_export(LDR_DATA_TABLE_ENTRY* const ldr_entry, const std::string_view name, const bool notify = true);

export namespace fd
{
    using ::find_export;
    using ::on_export_found;
} // namespace fd
