module;

#include <string_view>

export module cheat.netvars;
export import :core;
import nstd.mem.address;

namespace cheat::netvars
{
	nstd::mem::basic_address<const void> apply_offset(const void* thisptr, const std::string_view table, const std::string_view item) noexcept;
}
