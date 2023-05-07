#pragma once

#include "table.h"

#define FD_NETVARS_DT_MERGE

namespace fd
{
struct netvar_tables : std::vector<netvar_table>
{
    pointer find(std::string_view name);
    const_pointer find(std::string_view name) const;

    [[deprecated]]
    size_t index_of(const_pointer table) const;
    [[deprecated]]
    void sort(size_t index);
    void on_item_added(const_reference table) const;
};

class netvar_tables_ordered : public netvar_tables
{
    using sort_value = std::conditional_t<std::ranges::random_access_range<netvar_tables>, size_t, const_pointer>;

    std::vector<sort_value> sort_reqests_;

    using netvar_tables::sort;

  public:
    netvar_tables_ordered();

    void request_sort(const_pointer table);
    void sort();
};

} // namespace fd