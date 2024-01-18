#include "address.h"

namespace fd
{
void* resolve_relative_address(void const* address, ptrdiff_t const rva_offset, ptrdiff_t const rip_offset) noexcept
{
    auto const rva = *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(address) + rva_offset);
    auto const rip = reinterpret_cast<uintptr_t>(address) + rip_offset;

    return reinterpret_cast<void*>(rva + rip);
}

void* get_absolute_address(void const* relative_address, ptrdiff_t const pre_offset, ptrdiff_t const post_offset) noexcept
{
    auto addr = reinterpret_cast<uintptr_t>(relative_address);

    addr += pre_offset;
    addr += sizeof(int32_t) + *reinterpret_cast<int32_t const*>(addr);
    addr += post_offset;

    return reinterpret_cast<void*>(addr);
}
} // namespace fd