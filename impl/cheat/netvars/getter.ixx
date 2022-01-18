
module;

#include <string_view>

export module cheat.netvars_getter;

export namespace cheat
{
	size_t get_netvar_offset(const std::string_view& table, const std::string_view& item);
}