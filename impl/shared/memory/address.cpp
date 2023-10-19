#include "memory/address.h"

namespace fd
{
void* resolve_relative_address(void* address, uint32_t const rva_offset, uint32_t const rip_offset)
{
    auto const rva = *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(address) + rva_offset);
    auto const rip = reinterpret_cast<uintptr_t>(address) + rip_offset;

    return reinterpret_cast<void*>(rva + rip);
}
}