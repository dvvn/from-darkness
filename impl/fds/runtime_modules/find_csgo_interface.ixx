module;

#include <fds/core/event_fwd.h>

#include <windows.h>
#include <winternl.h>

#include <string_view>

export module fds.rt_modules:find_csgo_interface;

FDS_EVENT_FWD(on_csgo_interface_found, const LDR_DATA_TABLE_ENTRY* /* library name */, std::string_view /*interface name*/, const void* /*interface ptr*/);

// void* find_csgo_interface(LDR_DATA_TABLE_ENTRY* const ldr_entry, const std::string_view name, const bool notify = true);
void* find_csgo_interface(const void* create_interface_fn, const std::string_view name, LDR_DATA_TABLE_ENTRY* const ldr_entry_for_notification /* = nullptr */);

export namespace fds
{
    using ::find_csgo_interface;
    using ::on_csgo_interface_found;
} // namespace fds
