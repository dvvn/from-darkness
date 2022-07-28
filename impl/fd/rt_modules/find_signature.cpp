module;

#include <fd/assert.h>

#include <windows.h>
#include <winternl.h>

module fd.rt_modules:find_signature;
import :library_info;
import fd.signature;

uint8_t* find_signature(LDR_DATA_TABLE_ENTRY* const ldr_entry, const fd::string_view sig, const bool notify)
{
    if (!ldr_entry)
        return nullptr;

    const auto memory_span = fd::dos_nt(ldr_entry).read();
    const fd::signature_finder finder(memory_span.data(), memory_span.size());
    const auto result = finder(sig);

    if (notify)
        fd::library_info(ldr_entry).log("signature", sig, result);
    return (uint8_t*)result;
}
