module;

#include <cstddef>

module cheat.netvars;

_Address cheat::netvars::apply_offset(const void* thisptr, const size_t offset) noexcept
{
	const _Address addr = thisptr;
	return addr + offset;
}