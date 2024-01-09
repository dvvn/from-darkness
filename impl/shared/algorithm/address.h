#pragma once

#include <cstdint>

namespace fd
{
/// resolve rip relative address
/// @param[in] address as byte for the address we want to resolve
/// @param[in] rva_offset offset of the relative address
/// @param[in] rip_offset offset of the instruction pointer
/// @returns: pointer to resolved address
void* resolve_relative_address(void const* address, ptrdiff_t rva_offset, ptrdiff_t rip_offset) noexcept;

/// get absolute address from relative address
/// @param[in] relative_address pointer to relative address, e.g. destination address from JMP, JE, JNE and others instructions
/// @param[in] pre_offset offset before relative address
/// @param[in] post_offset offset after relative address
/// @returns: pointer to absolute address
void* get_absolute_address(void const* relative_address, ptrdiff_t pre_offset = 0, ptrdiff_t post_offset = 0) noexcept;
} // namespace fd
