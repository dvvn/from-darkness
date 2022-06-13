module;

#include <fds/core/assert.h>
#include <fds/core/callback_impl.h>

#include <windows.h>
#include <winternl.h>

#include <string_view>

module fds.rt_modules:find_signature;
import :helpers;
import fds.mem_block;

FDS_CALLBACK_BIND(on_signature_found);

uint8_t* find_signature(LDR_DATA_TABLE_ENTRY* const ldr_entry, const std::string_view sig, const bool notify)
{
    const auto [dos, nt] = fds::dos_nt(ldr_entry);

    const fds::mem_block         mem(dos.get<uint8_t*>(), nt->OptionalHeader.SizeOfImage);
    const fds::unknown_signature bytes(sig.data(), sig.size());

    const auto result = mem.find_block(bytes).data();
    if (notify)
        std::invoke(on_signature_found, ldr_entry, sig, result);
    return result;
}
