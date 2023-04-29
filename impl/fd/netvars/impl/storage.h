#pragma once

#include "generator/classes.h"
#include "generator/log.h"
#include "tables.h"

#include <fd/valve/client_class.h>
#include <fd/valve/data_map.h>

namespace fd
{
class netvars_storage final
{
    netvar_tables internal_;
    netvar_tables_ordered data_;

  public:
    void process(valve::client_class *cclass);
    void process(valve::data_map *root_map);

    void write(netvar_log &log);
    void write(netvar_classes &data);

    size_t get_offset(std::string_view class_name, std::string_view name) const;
    void clear();
};
} // namespace fd