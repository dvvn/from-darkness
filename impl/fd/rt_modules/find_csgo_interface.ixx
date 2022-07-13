module;

#include <windows.h>
#include <winternl.h>

#include <string_view>

export module fd.rt_modules:find_csgo_interface;

void on_csgo_interface_found(const LDR_DATA_TABLE_ENTRY* library_name, std::string_view interface_name, const void* interface_ptr);

// void* find_csgo_interface(LDR_DATA_TABLE_ENTRY* const ldr_entry, const std::string_view name, const bool notify = true);
void* find_csgo_interface(const void* create_interface_fn, const std::string_view name, LDR_DATA_TABLE_ENTRY* const ldr_entry_for_notification /* = nullptr */);

export namespace fd
{
    using ::find_csgo_interface;
    using ::on_csgo_interface_found;
} // namespace fd
