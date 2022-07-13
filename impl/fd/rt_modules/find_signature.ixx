module;

#include <windows.h>
#include <winternl.h>

#include <string_view>

export module fd.rt_modules:find_signature;

uint8_t* find_signature(LDR_DATA_TABLE_ENTRY* const ldr_entry, const std::string_view sig, const bool notify = true);

export namespace fd
{
    using ::find_signature;
} // namespace fd
