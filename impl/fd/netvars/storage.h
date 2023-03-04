#pragma once

#include <fd/netvars/classes.h>
#include <fd/netvars/log.h>
#include <fd/netvars/tables.h>
#include <fd/valve/client_class.h>
#include <fd/valve/data_map.h>

namespace fd
{
class netvars_storage final
{
    netvar_tables         internal_;
    netvar_tables_ordered data_;

  public:
    void iterate_client_class(valve::client_class* cclass);
    void iterate_datamap(valve::data_map* rootMap);

    void log_netvars(netvars_log& log);
    void generate_classes(netvars_classes& data);

    size_t get_offset(std::string_view className, std::string_view name) const;
    void   clear();
};
} // namespace fd