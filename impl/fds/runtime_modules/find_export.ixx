module;

#include <fds/runtime_modules/notification_fwd.h>

#include <windows.h>
#include <winternl.h>

#include <string_view>

export module fds.rt_modules:find_export;

FDS_RTM_NOTIFICATION(on_export_found, const LDR_DATA_TABLE_ENTRY* /* library name */, std::string_view /*export name*/, void* /*export ptr*/);

void* find_export(LDR_DATA_TABLE_ENTRY* const ldr_entry, const std::string_view name, const bool notify = true);

export namespace fds
{
    using ::find_export;
    using ::on_export_found;
} // namespace fds
