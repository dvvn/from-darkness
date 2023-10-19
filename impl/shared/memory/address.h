#pragma once
#include <cstdint>

namespace fd
{
/// resolve rip relative address
/// @param[in] address as byte for the address we want to resolve
/// @param[in] rva_offset offset of the relative address
/// @param[in] rip_offset offset of the instruction pointer
/// @returns: pointer to resolved address
void* resolve_relative_address(void* address, uint32_t rva_offset, uint32_t rip_offset);

} // namespace fd
