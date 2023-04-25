﻿#pragma once

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
    void iterate_client_class(valve::client_class *cclass);
    void iterate_datamap(valve::data_map *root_map);

    void log_netvars(netvar_log &log);
    void generate_classes(netvar_classes &data);

    size_t get_offset(std::string_view class_name, std::string_view name) const;
    void clear();
};
} // namespace fd