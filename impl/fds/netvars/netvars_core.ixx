module;

#include <string_view>

export module fds.netvars.core;

export namespace fds::netvars
{
    size_t get_offset(const std::string_view table, const std::string_view item);
    void write_logs();
} // namespace fds::netvars
