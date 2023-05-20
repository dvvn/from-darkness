#pragma once

#include "table.h"

#define FD_NETVARS_DT_MERGE

namespace fd
{
struct netvar_tables : std::vector<netvar_table>
{
    
    void on_item_added(const_reference table) const;
};

class netvar_tables_ordered : public netvar_tables
{
    std::vector<size_t /*offset or hash*/> sort_reqests_;

  public:
    netvar_tables_ordered();

    void request_sort(const_iterator table);
    void sort();
};

} // namespace fd