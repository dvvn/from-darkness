module;

export module fd.netvars.core;
import fd.string;

export namespace fd::netvars
{
    size_t get_offset(const fd::string_view table, const fd::string_view item);
    void write_logs();
} // namespace fd::netvars
