module;

#include <string_view>

export module cheat.netvars:core;

namespace cheat::netvars
{
	size_t get_offset(const std::string_view table, const std::string_view item) noexcept;
	void write_logs( ) noexcept;
}
