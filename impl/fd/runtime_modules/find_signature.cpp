module;

#include <fd/assert.h>
#include <fd/callback_impl.h>

#include <windows.h>
#include <winternl.h>

#include <string_view>

module fd.rt_modules:find_signature;
import :helpers;
import fd.mem_block;

FD_CALLBACK_BIND(on_signature_found);

uint8_t* find_signature(LDR_DATA_TABLE_ENTRY* const ldr_entry, const std::string_view sig, const bool notify)
{
    if (!ldr_entry)
        return nullptr;

    const auto [dos, nt] = fd::dos_nt(ldr_entry);

    const fd::mem_block         mem(dos.get<uint8_t*>(), nt->OptionalHeader.SizeOfImage);
    const fd::unknown_signature bytes(sig.data(), sig.size());

    const auto result = mem.find_block(bytes).data();
    if (notify)
        std::invoke(on_signature_found, ldr_entry, sig, result);
    return result;
}
