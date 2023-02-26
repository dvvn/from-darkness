#pragma once

#include <fd/netvars/basic_storage.h>
#include <fd/netvars/classes.h>
#include <fd/netvars/log.h>
#include <fd/netvars/tables.h>
#include <fd/valve/client_class.h>
#include <fd/valve/data_map.h>

namespace fd
{
class netvars_storage final : public basic_netvars_storage
{
    netvar_tables         internal_;
    netvar_tables_ordered data_;

  public:
    void iterate_client_class(valve::client_class const* cclass);
    void iterate_datamap(valve::data_map const* rootMap);

    void log_netvars(netvars_log& log);
    void generate_classes(netvars_classes& data);

    size_t get_offset(std::string_view className, std::string_view name) const override;
    void   clear();
};
} // namespace fd