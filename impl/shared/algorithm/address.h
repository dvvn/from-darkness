#pragma once

#include <cstdint>

namespace fd
{
/// resolve rip relative address
/// @param[in] address as byte for the address we want to resolve
/// @param[in] rva_offset offset of the relative address
/// @param[in] rip_offset offset of the instruction pointer
/// @returns: pointer to resolved address
inline void* resolve_relative_address(void const* address, ptrdiff_t const rva_offset, ptrdiff_t const rip_offset) noexcept
{
    auto const rva = *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(address) + rva_offset);
    auto const rip = reinterpret_cast<uintptr_t>(address) + rip_offset;

    return reinterpret_cast<void*>(rva + rip);
}

/// get absolute address from relative address
/// @param[in] relative_address pointer to relative address, e.g. destination address from JMP, JE, JNE and others instructions
/// @param[in] pre_offset offset before relative address
/// @param[in] post_offset offset after relative address
/// @returns: pointer to absolute address
inline void* get_absolute_address(void const* relative_address, ptrdiff_t const pre_offset = 0, ptrdiff_t const post_offset = 0) noexcept
{
    auto addr = reinterpret_cast<uintptr_t>(relative_address);

    addr += pre_offset;
    addr += sizeof(int32_t) + *reinterpret_cast<int32_t const*>(addr);
    addr += post_offset;

    return reinterpret_cast<void*>(addr);
}
} // namespace fd
