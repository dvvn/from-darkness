module;

#include <fd/rt_modules/winapi_fwd.h>

export module fd.rt_modules:find_csgo_interface;
export import fd.string;

// void* find_csgo_interface(LDR_DATA_TABLE_ENTRY* const ldr_entry, const fd::string_view name, const bool notify = true);
void* find_csgo_interface(const void* create_interface_fn, const fd::string_view name, LDR_DATA_TABLE_ENTRY* const ldr_entry_for_notification /* = nullptr */);

export namespace fd
{
    using ::find_csgo_interface;
}
