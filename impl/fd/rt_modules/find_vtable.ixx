module;

#include <windows.h>
#include <winternl.h>

export module fd.rt_modules:find_vtable;
export import fd.string;

void* find_vtable(LDR_DATA_TABLE_ENTRY* const ldr_entry, const fd::string_view name, const bool notify = true);

export namespace fd
{
    using ::find_vtable;
} // namespace fd
