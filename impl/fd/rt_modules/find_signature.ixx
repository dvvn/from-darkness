module;

#include <fd/rt_modules/winapi_fwd.h>

#include <cstdint>

export module fd.rt_modules:find_signature;
export import fd.string;

uint8_t* find_signature(LDR_DATA_TABLE_ENTRY* const ldr_entry, const fd::string_view sig, const bool notify = true);

export namespace fd
{
    using ::find_signature;
} // namespace fd
