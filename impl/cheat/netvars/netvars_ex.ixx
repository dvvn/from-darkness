module;

#include <string_view>

export module cheat.netvars.ex;
import cheat.netvars;
import nstd.mem.address;

namespace cheat::netvars
{
	nstd::mem::basic_address<void> apply_offset(const void* thisptr, const std::string_view table, const std::string_view item) noexcept;
}
