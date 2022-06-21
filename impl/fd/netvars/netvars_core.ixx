module;

#include <string_view>

export module fd.netvars.core;

export namespace fd::netvars
{
    size_t get_offset(const std::string_view table, const std::string_view item);
    void write_logs();
} // namespace fd::netvars
