module;

#include <string_view>

module cheat.netvars;

using namespace cheat;
using nstd::mem::basic_address;

basic_address<const void> netvars::apply_offset(const void* thisptr, const std::string_view table, const std::string_view item) noexcept
{
	const auto offset = get_offset(table, item);
	const basic_address addr = thisptr;
	return addr + offset;
}