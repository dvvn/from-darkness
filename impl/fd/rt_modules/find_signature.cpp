module;

#include <fd/assert.h>

#include <windows.h>
#include <winternl.h>

module fd.rt_modules:find_signature;
import :helpers;
import :library_info;
import fd.signature;

uint8_t* find_signature(LDR_DATA_TABLE_ENTRY* const ldr_entry, const fd::string_view sig, const bool notify)
{
    if (!ldr_entry)
        return nullptr;

    const auto [dos, nt] = fd::dos_nt(ldr_entry);

    /* const fd::mem_block         mem(dos.get<uint8_t*>(), nt->OptionalHeader.SizeOfImage);
    const fd::unknown_signature bytes(sig.data(), sig.size());

    const auto result = mem.find_block(bytes).data(); */

    const fd::signature_finder finder(dos.get<uint8_t*>(), nt->OptionalHeader.SizeOfImage);
    const auto result = finder(sig);

    if (notify)
        fd::library_info(ldr_entry).log("signature", sig, result);
    return (uint8_t*)result;
}
