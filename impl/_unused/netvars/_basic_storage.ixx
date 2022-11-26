module;

#include <vector>

export module fd.netvars.basic_storage;
export import fd.netvars.table;

using fd::netvars::netvar_table;

using hashed_string_view = fd::string_view;
using hashed_string      = fd::string;

struct basic_storage : std::vector<netvar_table>
{
    const netvar_table* find(const hashed_string_view name) const;
    netvar_table* find(const hashed_string_view name);

    netvar_table* add(netvar_table&& table);
    netvar_table* add(hashed_string&& name);
};

export namespace fd::netvars
{
    using ::basic_storage;
}
